#pragma once

namespace details {

template <typename, typename>
struct TupleCat;

template <typename... First, typename... Second>
struct TupleCat<std::tuple<First...>, std::tuple<Second...>> {
  using Type = std::tuple<First..., Second...>;
};

template <typename Head, typename... Tail>
struct FieldsToColumns {
  using Type = typename TupleCat<std::tuple<std::vector<Head>>,
                                 typename FieldsToColumns<Tail...>::Type>::Type;
};

template <typename Head>
struct FieldsToColumns<Head> {
  using Type = std::tuple<std::vector<Head>>;
};

template <typename ColumnsTuple, typename RowTuple, size_t... Indices>
void push_row_impl(ColumnsTuple &t1, RowTuple &&t2,
                   std::integer_sequence<size_t, Indices...>) {
  auto emplace_wrapper = [](auto &container, auto v) {
    container.emplace_back(std::forward<decltype(v)>(v));
    return 0;
  };
  (void)std::initializer_list<int>{
      emplace_wrapper(std::get<Indices>(t1), (std::get<Indices>(t2)))...};
}

template <typename Tuple, typename Func, size_t... Indices>
auto tuple_transform_impl(const Tuple &t, Func f,
                          std::index_sequence<Indices...>) {
  return std::forward_as_tuple(f(std::get<Indices>(t))...);
}

template <typename... Fields, typename Func, size_t... Indices>
auto tuple_transform(const std::tuple<Fields...> &t, Func f) {
  return tuple_transform_impl(t, std::move(f),
                              std::index_sequence_for<Fields...>{});
}

} // namespace details

template <typename... Fields>
class Table {
private:
  details::FieldsToColumns<Fields...>::Type columns_;
  size_t size_{0};

  static Table<Fields...> from_columns_impl(
      typename details::FieldsToColumns<Fields...>::Type &&columns) {
    return Table<Fields...>(std::forward<decltype(columns)>(columns));
  }

public:
  Table() {
  }

  explicit Table(decltype(columns_) &&columns)
      : columns_(std::forward<decltype(columns_)>(columns)) {
    size_ = std::get<0>(columns_).size();
    // std::apply([size = this->size_](auto &col) { assert(col.size() == size);
    // },
    //            columns_);
  }

  template <int Index>
  const auto &get_col() const {
    return std::get<Index>(columns_);
  }

  void insert_row(Fields... fields) {
    details::push_row_impl(
        columns_, std::forward_as_tuple(std::forward<Fields>(fields)...),
        std::index_sequence_for<Fields...>{});
    ++size_;
  }

  size_t size() const {
    return size_;
  }

  auto get_row(size_t index) const {
    assert(index < size_);
    return details::tuple_transform(
        columns_, [index](auto &col) -> auto & { return col[index]; });
  }

  template <typename... Columns>
  static Table<Fields...> from_columns(Columns &&...columns) {
    return from_columns_impl(
        std::forward_as_tuple(std::forward<Columns>(columns)...));
  }
};