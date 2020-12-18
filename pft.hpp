// Copyright 2020 Ioannis Angelis <john_agelis@hotmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ============================================================
//
// ChangeLog:
//   0.0.5    map, take
//   0.0.4    remove Particle,
//            println for std::vector<T>
//   0.0.3    vec_from_range
//   0.0.2    pop, drop, readlines
//   0.0.1    Maybe<T>, StringView,
//            Particles_t, Matrix
//
// ============================================================

#ifndef PFT_H_
#define PFT_H_

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring> // for memset
#include <deque>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#ifdef PFT_USE_ROOT
#include <TLorentzVector.h>
#include <TMath.h>
#endif

// Unsigned
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// Signed
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Floating point
using f32 = float;
using f64 = double;

namespace pft {
// Maybe and StringView are based on https://github.com/rexim/aids
//////////////////////////////////////////////////
// Maybe
//////////////////////////////////////////////////
template <typename T>
struct Maybe {
  bool has_value;
  T unwrap;

  Maybe() : has_value(false) {}
  Maybe(bool f, T& val) : has_value(f), unwrap(val) {}
  Maybe(bool f, T&& val) : has_value(f), unwrap(std::move(val)) {}

  bool operator!=(const Maybe<T>& that) const { return !(*this == that); }

  bool operator==(const Maybe<T>& that) const {
    if (this->has_value && that.has_value) {
      return this->unwrap == that.unwrap;
    }

    return !this->has_value && !that.has_value;
  }
};

//////////////////////////////////////////////////
// StringView
//////////////////////////////////////////////////
struct StringView : public std::string_view {

  StringView() : std::string_view("") {}
  StringView(const std ::string& s) : std::string_view(s) {}
  StringView(const std ::string_view& s) : std::string_view(s) {}
  StringView(std ::string&& s) : std::string_view(std::move(s)) {}
  StringView(std ::string_view&& s) : std::string_view(std::move(s)) {}
  StringView(const char* s) : std::string_view(s) {}
  StringView(const char* s, size_t l) : std::string_view(s, l) {}

  StringView chop(size_t n) {
    if (this->size() > n) {
      return this->substr(n);
    } else {
      return {};
    }
  }

  StringView chop_by_delim(char delim) {
    size_t i = 0;
    while (i < this->size() && this->data()[i] != delim)
      ++i;
    StringView result = {this->data(), i};
    this->remove_prefix(i + 1);
    return result;
  }

  std::string as_string() const { return std::string(*this); }
};

//////////////////////////////////////////////////
// Particles struct usefull for Geant4
//////////////////////////////////////////////////
struct Particles_t {
  std::vector<i32> det_id;
  std::vector<i32> parent_id;
  std::vector<i32> trid;
  std::vector<f64> times;
  std::vector<f64> edep;
  std::vector<f64> energy;
  std::vector<f64> posX;
  std::vector<f64> posY;
  std::vector<f64> posZ;
  std::vector<f64> theta;
  std::vector<f64> phi;
  std::vector<f64> trlen;
  std::vector<i32> n_secondaries;

  void Reserve(const size_t nparticles) {
    det_id.reserve(nparticles);
    parent_id.reserve(nparticles);
    trid.reserve(nparticles);
    times.reserve(nparticles);
    edep.reserve(nparticles);
    energy.reserve(nparticles);
    posX.reserve(nparticles);
    posY.reserve(nparticles);
    posZ.reserve(nparticles);
    theta.reserve(nparticles);
    phi.reserve(nparticles);
    trlen.reserve(nparticles);
    n_secondaries.reserve(nparticles);
  }

  void ClearVecs() {
    det_id.clear();
    parent_id.clear();
    trid.clear();
    times.clear();
    edep.clear();
    energy.clear();
    posX.clear();
    posY.clear();
    posZ.clear();
    theta.clear();
    phi.clear();
    trlen.clear();
    n_secondaries.clear();
  }
};
} // namespace pft

namespace pft {

// StringView utilities
StringView operator""_sv(const char* data, size_t count) {
  return {data, count};
}
static inline StringView& trimr(StringView& s) {
  auto i = s.find_last_not_of(" \t\n\r\f\v");
  if (i != std::string_view::npos)
    s = s.substr(0, i + 1);

  return s;
}

static inline StringView& triml(StringView& s) {
  auto i = s.find_first_not_of(" \t\n\r\f\v");
  if (i != std::string_view::npos)
    s.remove_prefix(i);

  return s;
}

static inline std::vector<StringView> split_by(StringView view, char delim) {
  view = trimr(view);
  std::vector<StringView> ret;
  while (view.size() > 0) {
    auto len = view.find(delim);
    if (len == std::string_view::npos) {
      ret.push_back(view);
      break;
    }
    ret.push_back(view.substr(0, len));
    view = view.substr(len + 1);
  }
  return ret;
}

static inline std::vector<StringView> split_by(std::string& str, char delim) {
  std::vector<StringView> vec;
  StringView temp{str.data(), str.size()};
  StringView aug = {};
  while (0 < temp.size()) {
    aug = temp.chop_by_delim(delim);
    vec.emplace_back(aug.data(), aug.size());
  }
  return vec;
}

static inline StringView cstr_as_sv(const char* cstr) {
  return {cstr, strlen(cstr)};
}

static inline StringView string_as_sv(const std::string& s) {
  return {s.data(), s.length()};
}

static inline i32 to_int(StringView s) { return std::stoi(std::string(s)); }

static inline Maybe<StringView> read_file_as_string_view(const char* filename) {
  FILE* f = fopen(filename, "rb");
  if (!f)
    return {};

  int err = fseek(f, 0, SEEK_END);
  if (err < 0)
    return {};

  long size = ftell(f);
  if (size < 0)
    return {};

  err = fseek(f, 0, SEEK_SET);
  if (err < 0)
    return {};

  auto data = malloc(size);
  if (!data)
    return {};

  size_t read_size = fread(data, 1, size, f);
  if (read_size != (size_t)size && ferror(f))
    return {};

  fclose(f);
  return {true, {static_cast<const char*>(data), static_cast<size_t>(size)}};
}

static inline std::vector<StringView> readlines(const char* filename,
                                                const char delim = '\n') {

  auto file   = read_file_as_string_view(filename);
  auto result = split_by(file.unwrap, delim);
  return result;
}

template <typename T>
void pop(std::vector<T>& vec, size_t elements) {
  vec.erase(vec.begin(), vec.begin() + elements);
}

template <typename T>
void drop(std::vector<T>& vec, size_t elements) {
  for (size_t i = 0; i < elements; ++i) {
    vec.pop_back();
  }
}

template <typename T>
static inline std::vector<T> take(std::vector<T>& vec, size_t elements) {
  return {vec.begin(), vec.begin() + elements};
}

void ignore_header_lines(std::vector<StringView>& vec, int lines) {
  vec.erase(vec.begin(), vec.begin() + lines);
}

std::vector<float> as_floats(const std::vector<StringView>& vec) {
  std::vector<float> buffer(vec.size());

  for (size_t i = 0; i < vec.size(); ++i) {
    buffer[i] = std::stof(std::string(vec[i]));
  }
  return buffer;
}

//////////////////////////////////////////////////
// Printers
//////////////////////////////////////////////////
void print1(FILE* stream, char x) { fprintf(stream, "%c", x); }

void print1(FILE* stream, u32 x) { fprintf(stream, "%u", x); }
void print1(FILE* stream, u64 x) { fprintf(stream, "%lu", x); }
// void print1(FILE* stream, size_t x) { fprintf(stream, "%zu", x); }

void print1(FILE* stream, i16 x) { fprintf(stream, "%hd", x); }
void print1(FILE* stream, i32 x) { fprintf(stream, "%d", x); }
void print1(FILE* stream, i64 x) { fprintf(stream, "%ld", x); }

void print1(FILE* stream, f32 f) { fprintf(stream, "%f", f); }
void print1(FILE* stream, f64 f) { fprintf(stream, "%f", f); }

void print1(FILE* stream, StringView view) {
  fwrite(view.data(), 1, view.size(), stream);
}

template <typename... Types>
void println(FILE* stream, Types... args) {
  (print1(stream, args), ...);
  print1(stream, '\n');
}

// TODO: experimental Maybe printer
template <typename T>
void print1(FILE* stream, Maybe<T> m) {
  println(stream, "Maybe{ ", m.has_value, ", ", m.unwrap, " }");
}

template <typename T>
void println(FILE* stream, std::vector<T> args) {
  for (const auto& arg : args) {
    print1(stream, arg);
    print1(stream, '\n');
  }
}
//////////////////////////////////////////////////
// Matrix
//////////////////////////////////////////////////
template <typename T>
struct Matrix {
  size_t rows, cols;
  T* data{nullptr};

  Matrix(size_t r, size_t c) : rows(r), cols(c) { data = new T[rows * cols](); }
  Matrix(const Matrix<T>& m) : rows(m.rows), cols(m.cols), data(m.data) {}
  Matrix(Matrix<T>&& m) : rows(m.rows), cols(m.cols), data(std::move(m.data)) {}

  T& operator()(size_t i, size_t j) { return data[j + i * cols]; }

  const T& operator()(size_t i, size_t j) const { return data[j + i * cols]; }

  void diagonal(T d = (T)1.0) {
    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        if (i == j) {
          (*this)(i, j) = d;
        }
      }
    }
  }

  T trace() {
    T tr = 0;
    if (cols == rows) {
      for (size_t i = 0; i < cols; ++i) {
        tr += ((*this)(i, i));
      }
      return tr;
    } else {
      return std::numeric_limits<T>::quiet_NaN();
    }
  }

  Matrix<T> transpose() {
    Matrix<T> result(cols, rows);
    for (size_t i = 0; i < cols; ++i) {
      for (size_t j = 0; j < rows; ++j) {
        result(i, j) = ((*this)(j, i));
      }
    }
    return result;
  }
};

// TODO: make pretty printer for matrices
template <typename T>
void print1(FILE* stream, const Matrix<T>& mat) {
  for (size_t i = 0; i < mat.rows; ++i) {
    for (size_t j = 0; j < mat.cols; ++j) {
      print1(stream, mat(i, j));
      print1(stream, "  ");
    }
    fprintf(stream, "\n");
  }
}

//////////////////////////////////////////////////
// Utils
//////////////////////////////////////////////////
// http://reedbeta.com/blog/python-like-enumerate-in-cpp17/
template <typename T, typename TIter = decltype(std::begin(std::declval<T>())),
          typename = decltype(std::end(std::declval<T>()))>
constexpr auto enumerate(T&& iterable) {
  struct iterator {
    size_t i;
    TIter iter;
    bool operator!=(const iterator& other) const { return iter != other.iter; }
    void operator++() {
      ++i;
      ++iter;
    }
    auto operator*() const { return std::tie(i, *iter); }
  };
  struct iterable_wrapper {
    T iterable;
    auto begin() { return iterator{0, std::begin(iterable)}; }
    auto end() { return iterator{0, std::end(iterable)}; }
  };
  return iterable_wrapper{std::forward<T>(iterable)};
}

template <typename T, typename R, typename FoldOp>
R foldl(const R& i, const std::vector<T>& xs, FoldOp fn) {
  auto ret = i;
  for (const auto& x : xs) {
    ret = fn(ret, x);
  }
  return ret;
}

template <typename T>
static inline T sum(const std::vector<T>& xs) {
  return foldl(T(), xs, [](const T& a, const T& b) -> T { return a + b; });
}

template <typename T>
static inline T mean(const std::vector<T>& xs) {
  return sum(xs) / (xs.size() - 1);
}

template <typename T>
static inline T stdev(const std::vector<T>& xs) {
  T a    = 0;
  auto m = mean(xs);
  std::for_each(std::begin(xs), std::end(xs),
                [&](const T x) { a += (x - m) * (x - m); });

  return sqrt(a / (xs.size() - 1));
}
template <typename T, typename Op>
static inline auto map(Op fn, const std::vector<T>& input)
    -> std::vector<decltype(fn(input[0]))> {

  std::vector<decltype(fn(input[0]))> ret;
  ret.reserve(input.size());
  for (const auto& i : input) {
    ret.push_back(fn(i));
  }

  return ret;
}

template <typename T>
std::vector<T> vec_from_range(i64 low, i64 high) {
  size_t elements = 0;
  if (low < 0) {
    elements = high - low + 1;
  } else {
    elements = high + low + 1;
  }
  std::vector<T> v(elements);
  i64 i = low;
  for (auto& x : v) {
    x = (T)i;
    ++i;
  }
  return v;
}

//////////////////////////////////////////////////
// Arg Parse
//////////////////////////////////////////////////
struct ArgOption {
  std::string short_op;
  std::string long_op;
  std::string msg;
  bool accepts_value;
};

struct AParse {
  using Value = std::string;
  size_t nArgs;
  std::deque<char*> args;
  std::vector<ArgOption> flags;
  std::map<std::string, std::pair<ArgOption, std::string>> arg_table;

  AParse(int argc, char** a) : nArgs(argc) {

    for (size_t i = 0; i < nArgs; ++i) {
      args.push_back(a[i]);
    }
  }

  void PrintArgv() {
    for (size_t i = 1; i < nArgs; ++i) {
      // printf("%s\n", args[i]);
    }
  }

  void Parse() {
    if (nArgs == 1) {
      PrintUsage();
      return;
    }

    for (auto [i, arg] : ::pft::enumerate(args)) {
      for (size_t j = 0; j < flags.size(); ++j) {
        if (arg == flags[j].short_op || arg == flags[j].long_op) {
          if (flags[j].accepts_value) {
            if (i + 1 >= nArgs) {
              fprintf(stderr, "Missing argument for flag `%s` \n", arg);
              return;
            } else {
              arg_table.insert({flags[j].short_op, {flags[j], args[i + 1]}});
              args.pop_front();
            }
          } else {
            arg_table.insert({flags[j].short_op, {flags[j], ""}});
            args.pop_front();
            fprintf(stderr, "TODO: handle non-accepting value args\n");
          }
        }
      }
    }

    // fprintf(stdout, "[DBG]: %zu\n", options.size());
  }

  void Add(ArgOption opt) { flags.push_back(opt); }

  void PrintUsage() {
    for (auto& op : flags) {
      println(stdout, op.short_op, ", ", op.long_op, "\t:", op.msg);
    }
  }

  Maybe<std::string> value_of(std::string flag) {
    // TODO: check if flag exist
    auto val = arg_table.find(flag);

    if (val != arg_table.end()) {
      println(stdout, "[DBG]: ", val->second.second.c_str());
      return {val->second.first.accepts_value, val->second.second};
    } else {
      return {false, ""};
    }
  }
};
} // namespace pft

#endif // PFT_H_
