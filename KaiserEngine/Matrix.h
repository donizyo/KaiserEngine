#pragma once

template <typename _Ty, typename std::enable_if<std::is_floating_point<_Ty>::value>::type* = nullptr>
const _Ty
degrees2radians(_Ty degrees)
{
    static const auto pi = std::acos(-1);
    return static_cast<_Ty>(degrees / 180 * pi);
}


template <typename _Ty, int Width, int Height>
class Matrix
{
private:
    bool isTransposed;
    _Ty data[Width * Height];

    // pos = (row, column)
    int convert2index(const std::pair<int, int>& pos) const
    {
        return convert2index(pos.first, pos.second);
    }

    int convert2index(const int& row, const int& column) const
    {
        if (row < 0)
            throw std::domain_error("Invalid argument: row < 0!");
        if (row >= Height)
            throw std::domain_error("Invalid argument: row >= Height!");
        if (column < 0)
            throw std::domain_error("Invalid argument: column < 0!");
        if (column >= Width)
            throw std::domain_error("Invalid argument: column >= Width!");

        if (isTransposed)   return row * Width + column;    // row-first
        else                return row + column * Height;   // column-first
    }

public:
    Matrix(const bool& isTransposed = false)
        : isTransposed{ isTransposed }
        , data{ 0 }
    {
    }

    ~Matrix() {}

    constexpr bool IsVector() const
    {
        return Height == 1 && Width > 0
            || Width == 1 && Height > 0;
    }

    _Ty& operator[](const std::pair<int, int>& pos)
    {
        return data[convert2index(pos)];
    }

    void transpose()
    {
        isTransposed = !isTransposed;
    }

    // print internal data structure, regardless of whether the matrix is transposed or not
    const std::string ToString() const
    {
        std::stringstream ss;

        for (int row = 0; row < Height; row++)
        {
            ss << "| ";
            for (int column = 0; column < Width; column++)
            {
                ss << data[convert2index(row, column)]
                    << " ";
            }
            ss << "|"
                << std::endl;
        }

        return ss.str();
    }

public:

    // enables invoking `matrix[row][column]`

    class MatrixVector
    {
    private:
        const Matrix& parent;
        const int first;

    public:
        MatrixVector(const Matrix& matrix, const int& firstIndex)
            : parent{ matrix }
            , first{ firstIndex }
        {
        }

        _Ty& operator[](const int& secondIndex)
        {
            const int index = parent.convert2index(first, secondIndex);
            return const_cast<_Ty&>(parent.data[index]);
        }
    };

    MatrixVector operator[](const int& firstIndex)
    {
        return MatrixVector(*this, firstIndex);
    }
};

// squre matrix
template <typename _Ty, int N>
using MatrixQ = Matrix<_Ty, N, N>;

using Matrix3i = MatrixQ<int, 3>;
using Matrix4i = MatrixQ<int, 4>;
using Matrix3f = MatrixQ<float, 3>;
using Matrix4f = MatrixQ<float, 4>;
using Matrix3d = MatrixQ<double, 3>;
using Matrix4d = MatrixQ<double, 4>;
