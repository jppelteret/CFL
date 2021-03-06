#ifndef cfl_dealii_matrixfree_forms_h
#define cfl_dealii_matrixfree_forms_h

#include <array>
#include <iostream>
#include <string>
#include <utility>

#include <tuple>

#include <deal.II/base/exceptions.h>

#include <cfl/base/forms.h>
#include <cfl/base/traits.h>

#include <cfl/matrixfree/fefunctions.h>

namespace CFL
{
namespace dealii::MatrixFree
{
  template <typename... Types>
  class Forms;

  /**
   * A Form is an expression tested by a test function set.
   */
  template <class Test, class Expr, FormKind kind_of_form, typename NumberType = double>
  class Form final
  {
  public:
    using TestType = Test;
    const Test test;
    const Expr expr;

    static constexpr FormKind form_kind = kind_of_form;

    static constexpr unsigned int fe_number = Test::index;
    static constexpr bool integrate_value = Test::integration_flags.value;
    static constexpr bool integrate_value_exterior =
      (kind_of_form == FormKind::face) ? Test::integration_flags.value_exterior : false;
    static constexpr bool integrate_gradient = Test::integration_flags.gradient;
    static constexpr bool integrate_gradient_exterior =
      (kind_of_form == FormKind::face) ? Test::integration_flags.gradient_exterior : false;

    template <class OtherTest, class OtherExpr>
    explicit constexpr Form(const Base::Form<OtherTest, OtherExpr, kind_of_form, NumberType> f)
      : test(transform(f.test))
      , expr(transform(f.expr))
    {
    }

    static constexpr std::array<bool, 3>
    get_form_kinds(std::array<bool, 3> use_objects = std::array<bool, 3>{})
    {
      switch (form_kind)
      {
        case FormKind::cell:
          use_objects[0] = true;
          break;

        case FormKind::face:
          use_objects[1] = true;
          break;

        case FormKind::boundary:
          use_objects[2] = true;
          break;

        default:
          static_assert("Invalid FormKind!");
      }
      return use_objects;
    }

    template <class FEEvaluation>
    static void
    integrate(FEEvaluation& phi)
    {
      // only to be used if there is only one form!
      phi.template integrate<fe_number>(integrate_value, integrate_gradient);
    }

    template <class FEEvaluation>
    static void
    set_integration_flags(FEEvaluation& phi)
    {
      // only to be used if there is only one form!
      if constexpr(form_kind == FormKind::cell)
          phi.template set_integration_flags<fe_number>(integrate_value, integrate_gradient);
    }

    template <class FEEvaluation>
    static void
    set_integration_flags_face(FEEvaluation& phi)
    {
      // only to be used if there is only one form!
      if constexpr(form_kind == FormKind::face)
          phi.template set_integration_flags_face_and_boundary<fe_number>(
            integrate_value,
            integrate_value_exterior,
            integrate_gradient,
            integrate_gradient_exterior);
    }

    template <class FEEvaluation>
    static void
    set_integration_flags_boundary(FEEvaluation& phi)
    {
      // only to be used if there is only one form!
      if constexpr(form_kind == FormKind::boundary)
          phi.template set_integration_flags_face_and_boundary<fe_number>(
            integrate_value,
            integrate_value_exterior,
            integrate_gradient,
            integrate_gradient_exterior);
    }

    template <class FEEvaluation>
    void
    set_evaluation_flags(FEEvaluation& phi) const
    {
      // only to be used if there is only one form!
      if constexpr(form_kind == FormKind::cell) expr.set_evaluation_flags(phi);
    }

    template <class FEEvaluation>
    void
    set_evaluation_flags_face(FEEvaluation& phi) const
    {
      // only to be used if there is only one form!
      if constexpr(form_kind == FormKind::face || form_kind == FormKind::boundary)
          expr.set_evaluation_flags(phi);
    }

    template <class FEEvaluation>
    void evaluate([[maybe_unused]] FEEvaluation& phi, [[maybe_unused]] unsigned int q) const
    {
      if constexpr(form_kind == FormKind::cell)
        {
          // only to be used if there is only one form!
          const auto value = expr.value(phi, q);
          Test::submit(phi, q, value);
        }
    }

    template <class FEEvaluation>
    void evaluate_face([[maybe_unused]] FEEvaluation& phi, [[maybe_unused]] unsigned int q) const
    {
      if constexpr(form_kind == FormKind::face)
        {
          // only to be used if there is only one form!
          const auto value = expr.value(phi, q);
          Test::submit(phi, q, value);
        }
    }

    template <class FEEvaluation>
    void evaluate_boundary([[maybe_unused]] FEEvaluation& phi,
                           [[maybe_unused]] unsigned int q) const
    {
      if constexpr(form_kind == FormKind::boundary)
        {
          // only to be used if there is only one form!
          const auto value = expr.value(phi, q);
          Test::submit(phi, q, value);
        }
    }

    template <class FEEvaluation>
    auto
    value(FEEvaluation& phi, unsigned int q) const
    {
      return expr.value(phi, q);
    }

    template <class FEEvaluation, typename ValueType>
    static void
    submit(FEEvaluation& phi, unsigned int q, const ValueType& value)
    {
      Test::submit(phi, q, value);
    }
  };

  namespace internal
  {
    // allow for appending an object to an std::array object.
    template <typename T, std::size_t N, std::size_t... I>
    constexpr std::array<T, N + 1>
    append_aux(std::array<T, N> a, T t, std::index_sequence<I...>)
    {
      return std::array<T, N + 1>{ { a[I]..., t } };
    }

    template <typename T, std::size_t N>
    constexpr std::array<T, N + 1>
    append(std::array<T, N> a, T t)
    {
      return append_aux(a, t, std::make_index_sequence<N>());
    }
  }

  template <typename... Types>
  class Forms;

  template <>
  class Forms<>
  {
  public:
    static constexpr unsigned int number = 0; // unused
    explicit constexpr Forms(const Base::Forms<>&){};
  };

  template <typename FormType, typename... Types>
  class Forms<FormType, Types...> : public Forms<Types...>
  {
  public:
    static constexpr FormKind form_kind = FormType::form_kind;

    static constexpr bool integrate_value = FormType::integrate_value;
    static constexpr bool integrate_value_exterior =
      (form_kind == FormKind::face) ? FormType::integrate_value_exterior : false;
    static constexpr bool integrate_gradient = FormType::integrate_gradient;
    static constexpr bool integrate_gradient_exterior =
      (form_kind == FormKind::face) ? FormType::integrate_gradient_exterior : false;

    static constexpr unsigned int fe_number = FormType::fe_number;
    static constexpr unsigned int number = sizeof...(Types) == 0 ? 0 : Forms<Types...>::number + 1;

    template <class OtherType, class... OtherTypes,
              typename std::enable_if<sizeof...(OtherTypes) == sizeof...(Types)>::type* = nullptr>
    explicit constexpr Forms(const Base::Forms<OtherType, OtherTypes...>& f)
      : Forms<Types...>(static_cast<Base::Forms<OtherTypes...>>(f))
      , form(f.get_form())
    {
    }

    static constexpr std::array<bool, 3>
    get_form_kinds(std::array<bool, 3> use_objects = std::array<bool, 3>{})
    {
      switch (form_kind)
      {
        case FormKind::cell:
          use_objects[0] = true;
          break;

        case FormKind::face:
          use_objects[1] = true;
          break;

        case FormKind::boundary:
          use_objects[2] = true;
          break;

        default:
          static_assert("Invalid FormKind!");
      }
      if constexpr(sizeof...(Types) != 0) return Forms<Types...>::get_form_kinds(use_objects);
      else
        return use_objects;
    }

    template <class FEEvaluation>
    static void
    set_integration_flags(FEEvaluation& phi)
    {
      if constexpr(form_kind == FormKind::cell)
          phi.template set_integration_flags<fe_number>(integrate_value, integrate_gradient);

      if constexpr(sizeof...(Types) != 0) Forms<Types...>::set_integration_flags(phi);
    }

    template <class FEEvaluation>
    static void
    set_integration_flags_face(FEEvaluation& phi)
    {
      if constexpr(form_kind == FormKind::face)
          phi.template set_integration_flags_face_and_boundary<fe_number>(
            integrate_value,
            integrate_value_exterior,
            integrate_gradient,
            integrate_gradient_exterior);

      if constexpr(sizeof...(Types) != 0) Forms<Types...>::set_integration_flags_face(phi);
    }

    template <class FEEvaluation>
    static void
    set_integration_flags_boundary(FEEvaluation& phi)
    {
      if constexpr(form_kind == FormKind::boundary)
          phi.template set_integration_flags_face_and_boundary<fe_number>(
            integrate_value,
            integrate_value_exterior,
            integrate_gradient,
            integrate_gradient_exterior);

      if constexpr(sizeof...(Types) != 0) Forms<Types...>::set_integration_flags_boundary(phi);
    }

    template <class FEEvaluation>
    void
    set_evaluation_flags(FEEvaluation& phi) const
    {
      if constexpr(form_kind == FormKind::cell) form.expr.set_evaluation_flags(phi);

      if constexpr(sizeof...(Types) != 0) Forms<Types...>::set_evaluation_flags(phi);
    }

    template <class FEEvaluation>
    void
    set_evaluation_flags_face(FEEvaluation& phi) const
    {
      if constexpr(form_kind == FormKind::face || form_kind == FormKind::boundary)
          form.expr.set_evaluation_flags(phi);

      if constexpr(sizeof...(Types) != 0) Forms<Types...>::set_evaluation_flags_face(phi);
    }

    template <class FEEvaluation>
    void
    evaluate(FEEvaluation& phi, [[maybe_unused]] unsigned int q) const
    {
      if constexpr(form_kind == FormKind::cell)
        {
#ifdef DEBUG_OUTPUT
          std::cout << "expecting cell value from fe_number " << fe_number << std::endl;
#endif
          const auto value = form.value(phi, q);
          if constexpr(sizeof...(Types) != 0)
            {
#ifdef DEBUG_OUTPUT
              std::cout << "descending" << std::endl;
#endif
              Forms<Types...>::evaluate(phi, q);
            }
#ifdef DEBUG_OUTPUT
          std::cout << "expecting cell submit from fe_number " << fe_number << std::endl;
#endif
          form.submit(phi, q, value);
        }
      else if constexpr(sizeof...(Types) != 0) Forms<Types...>::evaluate(phi, q);
    }

    template <class FEEvaluation>
    void
    evaluate_face(FEEvaluation& phi, unsigned int q) const
    {
      if constexpr(form_kind == FormKind::face)
        {
#ifdef DEBUG_OUTPUT
          std::cout << "expecting face value from fe_number " << fe_number << std::endl;
#endif
          const auto value = form.value(phi, q);
          if constexpr(sizeof...(Types) != 0)
            {
#ifdef DEBUG_OUTPUT
              std::cout << "descending" << std::endl;
#endif
              Forms<Types...>::evaluate_face(phi, q);
            }
#ifdef DEBUG_OUTPUT
          std::cout << "expecting face submit from fe_number " << fe_number << std::endl;
#endif
          form.submit(phi, q, value);
        }
      else if constexpr(sizeof...(Types) != 0) Forms<Types...>::evaluate_face(phi, q);
    }

    template <class FEEvaluation>
    void
    evaluate_boundary(FEEvaluation& phi, [[maybe_unused]] unsigned int q) const
    {
      if constexpr(form_kind == FormKind::boundary)
        {
#ifdef DEBUG_OUTPUT
          std::cout << "expecting face value from fe_number " << fe_number << std::endl;
#endif
          const auto value = form.value(phi, q);
          if constexpr(sizeof...(Types) != 0)
            {
#ifdef DEBUG_OUTPUT
              std::cout << "descending" << std::endl;
#endif
              Forms<Types...>::evaluate_boundary(phi, q);
            }
#ifdef DEBUG_OUTPUT
          std::cout << "expecting face submit from fe_number " << fe_number << std::endl;
#endif
          form.submit(phi, q, value);
        }
      else if constexpr(sizeof...(Types) != 0) Forms<Types...>::evaluate_boundary(phi, q);
    }

    template <class FEEvaluation>
    static void
    integrate(FEEvaluation& phi)
    {
      phi.template integrate<fe_number>(integrate_value, integrate_gradient);
      if constexpr(sizeof...(Types) != 0) Forms<Types...>::integrate(phi);
    }

    constexpr const FormType&
    get_form() const
    {
      return form;
    }

  protected:
    template <unsigned int size, typename IntegrationFlags>
    static constexpr bool
    check_forms(
      const std::array<std::tuple<FormKind, unsigned int, std::remove_cv_t<IntegrationFlags>>,
                       size>& container =
        std::array<std::tuple<FormKind, unsigned int, std::remove_cv_t<IntegrationFlags>>, size>{})
    {
      constexpr IntegrationFlags integration_flags =
        decltype(std::declval<FormType>().test)::integration_flags;

      for (unsigned int i = number; i < size; ++i)
      {
        const auto& item = container.at(i);
        if (std::get<0>(item) == form_kind && std::get<1>(item) == fe_number &&
            ((std::get<2>(item)) & integration_flags))
          return false;
      }

      if constexpr(sizeof...(Types) != 0)
        {
          constexpr auto new_tuple = std::make_tuple(form_kind, fe_number, integration_flags);
          const auto new_container = internal::append(container, new_tuple);
          return Forms<Types...>::template check_forms<size + 1, IntegrationFlags>(new_container);
        }
      else
        return true;
    }

    static_assert(check_forms<number, decltype(FormType::TestType::integration_flags)>(),
                  "There are multiple forms that try to submit the same information!");

  private:
    const FormType form;
  };

  template <class Test, class Expr, FormKind kind_of_form, typename NumberType>
  constexpr auto
  transform(const Base::Form<Test, Expr, kind_of_form, NumberType>& f)
  {
    return Form<decltype(transform(std::declval<Test>())),
                decltype(transform(std::declval<Expr>())),
                kind_of_form>(f);
  }

  template <typename... Types>
  constexpr auto
  transform(const Base::Forms<Types...>& f)
  {
    return Forms<decltype(transform(std::declval<Types>()))...>(f);
  }
}
} // namespace CFL

#endif
