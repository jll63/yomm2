
struct matrix {
    virtual ~matrix() {}
    virtual double times(double) const = 0;
};

struct dense_matrix : matrix {
    virtual double times(double) const;
};

struct diagonal_matrix : matrix {
    virtual double times(double) const;
};
