
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
