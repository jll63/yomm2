#include <yorel/yomm2.hpp>

struct dense_matrix;
struct diagonal_matrix;

struct matrix {
    virtual ~matrix() {}
    virtual double times(double) const = 0;
    virtual double times(const matrix& other) const = 0;
    virtual double times2(const dense_matrix& other) const;
    virtual double times2(const diagonal_matrix& other) const;
};

struct dense_matrix : matrix {
    virtual double times(double) const;
    virtual double times(const matrix& other) const;
    virtual double times2(const dense_matrix& other) const;
};

struct diagonal_matrix : matrix {
    virtual double times(double) const;
    virtual double times(const matrix& other) const;
    virtual double times2(const diagonal_matrix& other) const;
};

YOMM2_DECLARE(double, times, double, yorel::yomm2::virtual_<const matrix&>);
YOMM2_DECLARE(double, times, yorel::yomm2::virtual_<const matrix&>, yorel::yomm2::virtual_<const matrix&>);

double call_virtual_function(double s, const matrix& m);
double call_uni_method(double s, const matrix& m);
