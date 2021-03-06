// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include <cfl/latex/evaluator.h>
#include <cfl/latex/fefunctions.h>
#include <cfl/latex/forms.h>

using namespace CFL;

int
main()
{
  CFL::Base::FEFunction<0, 2, 0> p;
  CFL::Base::FEFunction<0, 2, 1> q;
  CFL::Base::FEFunction<1, 2, 2> u;

  CFL::Base::TestFunction<0, 2, 0> phi;

  assert_is_summable(p, q);

  std::vector<std::string> function_names{ "p", "q", "u" };
  std::vector<std::string> test_names{ "\\phi" };

  Latex::Evaluator(Latex::transform(form(phi, p + q)), function_names, test_names).print(std::cout);
  Latex::Evaluator(Latex::transform(form(phi, p * q)), function_names, test_names).print(std::cout);
  Latex::Evaluator(Latex::transform(form(phi, 4 * p * q)), function_names, test_names)
    .print(std::cout);
  Latex::Evaluator(Latex::transform(form(grad(phi), grad(p) + grad(q))), function_names, test_names)
    .print(std::cout);
  Latex::Evaluator(Latex::transform(form(grad(grad(phi)), grad(grad(p)) + grad(grad(q)))),
                   function_names,
                   test_names)
    .print(std::cout);
  Latex::Evaluator(Latex::transform(form(grad(phi), u + grad(q))), function_names, test_names)
    .print(std::cout);
  // TODO
  // Latex::Evaluator(Latex::transform(form(grad(phi), p * grad(q))), function_names,
  // test_names).print(std::cout);
  // TODO
  // Latex::Evaluator(Latex::transform(form(grad(phi), u * p)), function_names,
  // test_names).print(std::cout);

  return 0;
}
