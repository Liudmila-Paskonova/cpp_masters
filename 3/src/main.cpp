#include <iostream>
#include "matrix/matrix.hpp"
#include "nth_power/nth_power.hpp"

int
main()
{
    using matrix::SquareMatrix;
    using namespace nth_power;

    SquareMatrix<int, 2> a{{{1, 2}, {3, 4}}};
    std::cout << a << '\n';

    auto a2 = nth_power::nth_power(a, 2);
    std::cout << a2 << '\n';

    SquareMatrix<int, 2> id(1);
    auto id3 = nth_power::nth_power(id, 3);
    std::cout << id3 << '\n';

    int scalar = 2;
    std::cout << nth_power::nth_power(scalar, 10) << '\n';

    return 0;
}
