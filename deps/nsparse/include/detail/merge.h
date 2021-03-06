#pragma once

#include <detail/merge_path.cuh>
#include <detail/meta.h>
#include <detail/util.h>

#include <thrust/device_ptr.h>
#include <thrust/device_vector.h>

namespace nsparse {

template <typename index_type>
struct unique_merge_functor_t {
  template <typename... Borders>
  void exec_merge_count(const thrust::device_vector<index_type>& rpt_a,
                        const thrust::device_vector<index_type>& col_a,
                        const thrust::device_vector<index_type>& rpt_b,
                        const thrust::device_vector<index_type>& col_b,
                        thrust::device_vector<index_type>& rpt_c,
                        const thrust::device_vector<index_type>& permutation_buffer,
                        const thrust::device_vector<index_type>& bin_offset,
                        const thrust::device_vector<index_type>& bin_size, std::tuple<Borders...>) {
    EXPAND_SIDE_EFFECTS(
        (bin_size[Borders::bin_index] > 0
             ? merge_path_count<index_type, Borders::config_t::block_size>
             <<<(index_type)bin_size[Borders::bin_index], Borders::config_t::block_size>>>(
                 rpt_a.data(), col_a.data(), rpt_b.data(), col_b.data(), rpt_c.data(),
                 permutation_buffer.data() + bin_offset[Borders::bin_index])
             : void()));
  }

  template <typename... Borders>
  void exec_merge_fill(const thrust::device_vector<index_type>& rpt_a,
                       const thrust::device_vector<index_type>& col_a,
                       const thrust::device_vector<index_type>& rpt_b,
                       const thrust::device_vector<index_type>& col_b,
                       const thrust::device_vector<index_type>& rpt_c,
                       thrust::device_vector<index_type>& col_c,
                       const thrust::device_vector<index_type>& permutation_buffer,
                       const thrust::device_vector<index_type>& bin_offset,
                       const thrust::device_vector<index_type>& bin_size, std::tuple<Borders...>) {
    EXPAND_SIDE_EFFECTS(
        (bin_size[Borders::bin_index] > 0
             ? merge_path_fill<index_type, Borders::config_t::block_size>
             <<<(index_type)bin_size[Borders::bin_index], Borders::config_t::block_size>>>(
                 rpt_a.data(), col_a.data(), rpt_b.data(), col_b.data(), rpt_c.data(), col_c.data(),
                 permutation_buffer.data() + bin_offset[Borders::bin_index])
             : void()));
  }

  template <typename... Borders>
  std::pair<thrust::device_vector<index_type>, thrust::device_vector<index_type>> operator()(
      const thrust::device_vector<index_type>& rpt_a,
      const thrust::device_vector<index_type>& col_a,
      const thrust::device_vector<index_type>& rpt_b,
      const thrust::device_vector<index_type>& col_b, std::tuple<Borders...>) {
    assert(rpt_a.size() == rpt_b.size());
    assert(rpt_a.size() > 0);

    constexpr size_t bin_count = sizeof...(Borders);
    constexpr size_t unused_bin = meta::max_bin<Borders...> + 1;

    auto rows = rpt_a.size() - 1;

    util::resize_and_fill_zeros(bin_size, bin_count);
    permutation_buffer.resize(rows);

    thrust::for_each(thrust::counting_iterator<index_type>(0),
                     thrust::counting_iterator<index_type>(rows),
                     [rpt_a = rpt_a.data(), rpt_b = rpt_b.data(),
                      row_per_bin = bin_size.data()] __device__(index_type rid) {
                       index_type items = rpt_a[rid + 1] - rpt_a[rid] + rpt_b[rid + 1] - rpt_b[rid];

                       size_t bin = meta::select_bin<Borders...>(items, unused_bin);
                       if (bin != unused_bin)
                         atomicAdd(row_per_bin.get() + bin, 1);
                     });

    thrust::device_vector<index_type> bin_offset(bin_count);
    thrust::exclusive_scan(bin_size.begin(), bin_size.end(), bin_offset.begin());

    thrust::fill(bin_size.begin(), bin_size.end(), 0);

    thrust::for_each(thrust::counting_iterator<index_type>(0),
                     thrust::counting_iterator<index_type>(rows),
                     [rpt_a = rpt_a.data(), rpt_b = rpt_b.data(), bin_size = bin_size.data(),
                      rows_in_bins = permutation_buffer.data(),
                      bin_offset = bin_offset.data()] __device__(index_type rid) {
                       index_type items = rpt_a[rid + 1] - rpt_a[rid] + rpt_b[rid + 1] - rpt_b[rid];

                       size_t bin = meta::select_bin<Borders...>(items, unused_bin);

                       if (bin == unused_bin)
                         return;

                       auto curr_bin_size = atomicAdd(bin_size.get() + bin, 1);
                       rows_in_bins[bin_offset[bin] + curr_bin_size] = rid;
                     });

    thrust::device_vector<index_type> rpt_c(rows + 1, 0);

    exec_merge_count(rpt_a, col_a, rpt_b, col_b, rpt_c, permutation_buffer, bin_offset, bin_size,
                     std::tuple<Borders...>{});

    thrust::exclusive_scan(rpt_c.begin(), rpt_c.end(), rpt_c.begin());

    thrust::device_vector<index_type> col_c(rpt_c.back());

    exec_merge_fill(rpt_a, col_a, rpt_b, col_b, rpt_c, col_c, permutation_buffer, bin_offset,
                    bin_size, std::tuple<Borders...>{});

    return {std::move(rpt_c), std::move(col_c)};
  }

 private:
  thrust::device_vector<index_type> bin_size;
  thrust::device_vector<index_type> permutation_buffer;
};

}  // namespace nsparse