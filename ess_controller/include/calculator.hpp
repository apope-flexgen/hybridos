#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <algorithm>
#include <regex>
#include <stack>

#include "asset.h"
#include "ess_utils.h"
#include "formatters.hpp"

// Namespace for common math operations
namespace Math
{
using Operator = std::string;
// The list of all operators supported for math operations
static std::vector<Operator> operators = { "+",   "-",    "*",     "/",   "%",  "avg",    "pctOf", "max",
                                           "min", "sqrt", "scale", "and", "or", "stddev", ">",     "<" };

inline double add(const std::vector<double>& operands)
{
    double result = 0;
    for (const double& val : operands)
    {
        result += val;
    }
    return result;
}

inline double subtract(const std::vector<double>& operands)
{
    double result = 0;
    bool firstVal = true;
    for (const double& val : operands)
    {
        if (firstVal)
        {
            result = val;
            firstVal = false;
        }
        else
            result -= val;
    }
    return result;
}

inline double multiply(const std::vector<double>& operands)
{
    double result = 1;
    for (const double& val : operands)
    {
        result *= val;
    }
    return result;
}

inline double divide(const std::vector<double>& operands)
{
    double result = 0;
    bool firstVal = true;
    for (const double& val : operands)
    {
        if (firstVal)
        {
            result = val;
            firstVal = false;
        }
        else if (val == 0)
        {
            if (0)
                FPS_PRINT_ERROR("Division by 0", NULL);
            if (0)
                ESSLogger::get().error("[zero division] on [{}] in [{}]", result, __func__);
            return 0;
        }
        else
            result /= val;
    }
    return result;
}

inline double modulus(const std::vector<double>& operands)
{
    double result = 0;
    bool firstVal = true;
    for (const double& val : operands)
    {
        if (firstVal)
        {
            result = val;
            firstVal = false;
        }
        else if (val == 0)
        {
            if (0)
                FPS_PRINT_ERROR("Modulus division by 0", NULL);
            if (0)
                ESSLogger::get().error("[modulus zero division] on [{}] in [{}]", result, __func__);
            return 0;
        }
        else
            result = fmod(result, val);
    }
    return result;
}

inline double average(const std::vector<double>& operands)
{
    double result = 0;
    int count = 0;
    for (const double& val : operands)
    {
        result += val;
        count++;
    }
    return count > 0 ? result / count : 0;
}

inline double percentageOf(const std::vector<double>& operands)
{
    double result = 0;
    double percentage = 0;
    bool firstVal = true;
    for (const double& val : operands)
    {
        if (firstVal)
        {
            result = val;
            firstVal = false;
        }
        else
            percentage += val;
    }
    percentage = percentage <= 1 ? percentage : 1;
    return result - (result * percentage);
}

inline double scale(double val, double scale)
{
    return scale > 0 ? val / scale : val;
}

inline double squareRoot(double val)
{
    return val > 0 ? sqrt(val) : 0;
}

inline double max(const std::vector<double>& operands)
{
    if (operands.size() <= 0)
    {
        FPS_PRINT_DEBUG("operands is empty", NULL);
        return 0;
    }
    return *std::max_element(std::begin(operands), std::end(operands));
}

inline double min(const std::vector<double>& operands)
{
    if (operands.size() <= 0)
    {
        FPS_PRINT_DEBUG("operands is empty", NULL);
        return 0;
    }
    return *std::min_element(std::begin(operands), std::end(operands));
}

// Standard deviation function
inline double stddev(const std::vector<double>& operands)
{
    if (operands.size() <= 0)
    {
        FPS_PRINT_WARN("operands is empty", NULL);
        return 0;
    }

    // Find the mean first
    double meanVal = Math::average(operands);

    double results = 0;
    for (const double& val : operands)
    {
        results += pow(val - meanVal, 2);
    }
    return sqrt(results / operands.size());
}
}  // namespace Math

// Namespace for validating and evaluating expressions
namespace Expression
{
using Operator = std::string;
using Precedence = int;
enum class Associativity
{
    none,
    leftAssociative,
    rightAssociative
};
struct OperatorInfo
{
    Precedence prec;
    Associativity assoc;
};

enum DataType
{
    NUMERIC,
    BOOL,
    STRING
};
struct Operand
{
    DataType type;
    double dval;
    bool bval;
    std::string sval;
};

struct Result
{
    DataType type;
    double dval;
    bool bval;
    std::string sval;
};

// The map of all operators and the corresponding precedence and associativity
// rules
static std::map<Operator, OperatorInfo> operatorMap = {
    { "!", { 12, Associativity::rightAssociative } }, { "not", { 12, Associativity::rightAssociative } },
    { "~", { 12, Associativity::rightAssociative } }, { "**", { 11, Associativity::rightAssociative } },
    { "*", { 10, Associativity::leftAssociative } },  { "/", { 10, Associativity::leftAssociative } },
    { "%", { 10, Associativity::leftAssociative } },  { "+", { 9, Associativity::leftAssociative } },
    { "-", { 9, Associativity::leftAssociative } },   { "<<", { 8, Associativity::leftAssociative } },
    { ">>", { 8, Associativity::leftAssociative } },  { "<", { 7, Associativity::leftAssociative } },
    { "<=", { 7, Associativity::leftAssociative } },  { ">", { 7, Associativity::leftAssociative } },
    { ">=", { 7, Associativity::leftAssociative } },  { "==", { 6, Associativity::leftAssociative } },
    { "!=", { 6, Associativity::leftAssociative } },  { "&", { 5, Associativity::leftAssociative } },
    { "^", { 4, Associativity::leftAssociative } },   { "|", { 3, Associativity::leftAssociative } },
    { "&&", { 2, Associativity::leftAssociative } },  { "and", { 2, Associativity::leftAssociative } },
    { "||", { 1, Associativity::leftAssociative } },  { "or", { 1, Associativity::leftAssociative } },
};

// The collection of functions that can be used in an expression
static std::vector<std::string> funcs = { "sqrt", "max", "min", "if" };

/**
 * @brief Checks if the given token is a number
 *
 * @param token the token to check
 * @return true if token is a number
 */
inline bool isNumber(const std::string& token)
{
    char* end = nullptr;
    double val = strtod(token.c_str(), &end);
    return end != token.c_str() && *end == '\0' && val != HUGE_VAL;
}

/**
 * @brief Checks if the given token is a boolean
 *
 * @param token the token to check
 * @return true if token is a boolean
 */
inline bool isBoolean(const std::string& token)
{
    return token == "true" || token == "false";
}

/**
 * @brief Checks if the given token is an operator
 *
 * @param token the token to check
 * @return true if token is an operator
 */
inline bool isOperator(const std::string& token)
{
    return operatorMap.find(token) != operatorMap.end();
}

/**
 * @brief Checks if the given token is a valid function
 *
 * @param token the token to check
 * @return true if token is a function
 */
inline bool isFunction(const std::string& token)
{
    return std::find(funcs.begin(), funcs.end(), token) != funcs.end();
}

/**
 * @brief Get the precedence level for a particular operator
 *
 * @param oper the operator to check precedence level for
 * @return the operator's precedence
 */
inline int getPrecedence(const std::string& oper)
{
    return operatorMap.find(oper) != operatorMap.end() ? operatorMap[oper].prec : -1;
}

/**
 * @brief Helper function for evaluating two operands in the stack
 * based on the operator
 *
 * @param operators the operators stack
 * @param operands the operands stack
 * @return true if operands are evaluated
 */
inline bool evaluate(std::stack<Expression::Operator>& operators, std::stack<Expression::Operand>& operands)
{
    // If the operators is empty, then we have an invalid operators stack
    if (operators.empty())
    {
        FPS_PRINT_DEBUG("The list of operators is empty", NULL);
        return false;
    }

    Expression::Operator oper = operators.top();

    // The following operators take at least one operand
    if (oper == "!" || oper == "not" || oper == "~")
    {
        if (operands.empty())
        {
            FPS_PRINT_DEBUG("The list of operands is empty", NULL);
            return false;
        }

        Expression::Operand operand = operands.top();
        operands.pop();

        // Logical operators
        if (oper == "!" || oper == "not")
        {
            bool val = (operand.type == Expression::DataType::NUMERIC) ? (bool)operand.dval : operand.bval;
            operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, !val, "" });
        }

        // Bitwise operator
        else if (oper == "~")
            operands.push(Expression::Operand{ Expression::DataType::NUMERIC, (double)~(int)operand.dval, false, "" });

        operators.pop();
        return true;
    }

    // The following operators take at least two operands
    if (operands.size() < 2)
    {
        FPS_PRINT_DEBUG(
            "The list of operators is empty. Two operands are needed "
            "to run logical, comparison, arithmetic, and bitwise "
            "operations",
            NULL);
        return false;
    }

    // Operator that takes two operands
    Expression::Operand operand2 = operands.top();
    operands.pop();
    Expression::Operand operand1 = operands.top();
    operands.pop();

    if (operand1.type != operand2.type)
    {
        if ((operand1.type == Expression::DataType::NUMERIC || operand1.type == Expression::DataType::BOOL) &&
            operand2.type == Expression::DataType::STRING)
        {
            FPS_PRINT_WARN("Type mismatch between operand 1 [{}:{}] and operand 2 [{}:string]",
                           operand1.type == Expression::DataType::NUMERIC ? operand1.dval : operand1.bval,
                           operand1.type == Expression::DataType::NUMERIC ? "numeric" : "boolean", operand2.sval);
            return false;
        }
        if ((operand2.type == Expression::DataType::NUMERIC || operand2.type == Expression::DataType::BOOL) &&
            operand1.type == Expression::DataType::STRING)
        {
            FPS_PRINT_WARN("Type mismatch between operand 1 [{}:{}] and operand 2 [{}:string]",
                           operand2.type == Expression::DataType::NUMERIC ? operand2.dval : operand2.bval,
                           operand2.type == Expression::DataType::NUMERIC ? "numeric" : "boolean", operand1.sval);
            return false;
        }
    }

    // Logical operators
    if (oper == "&&" || oper == "and")
    {
        bool val1 = (operand1.type == Expression::DataType::NUMERIC) ? (bool)operand1.dval : operand1.bval;
        bool val2 = (operand2.type == Expression::DataType::NUMERIC) ? (bool)operand2.dval : operand2.bval;
        operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, val1 && val2, "" });
    }
    else if (oper == "||" || oper == "or")
    {
        bool val1 = (operand1.type == Expression::DataType::NUMERIC) ? (bool)operand1.dval : operand1.bval;
        bool val2 = (operand2.type == Expression::DataType::NUMERIC) ? (bool)operand2.dval : operand2.bval;
        operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, val1 || val2, "" });
    }
    // Comparison operators
    else if (oper == "<")
        operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.dval < operand2.dval, "" });
    else if (oper == "<=")
        operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.dval <= operand2.dval, "" });
    else if (oper == ">")
        operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.dval > operand2.dval, "" });
    else if (oper == ">=")
        operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.dval >= operand2.dval, "" });
    else if (oper == "==")
    {
        if (operand1.type == Expression::DataType::STRING)
            operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.sval == operand2.sval, "" });
        else
            operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.dval == operand2.dval, "" });
    }
    else if (oper == "!=")
        if (operand1.type == Expression::DataType::STRING)
            operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.sval != operand2.sval, "" });
        else
            operands.push(Expression::Operand{ Expression::DataType::BOOL, 0, operand1.dval != operand2.dval, "" });

    // Arithmetic operators
    else if (oper == "+")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC, operand1.dval + operand2.dval, false, "" });
    else if (oper == "-")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC, operand1.dval - operand2.dval, false, "" });
    else if (oper == "*")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC, operand1.dval * operand2.dval, false, "" });
    else if (oper == "/")
    {
        if (operand2.dval == 0)
        {
            if (0)
                FPS_PRINT_ERROR("Division by 0", NULL);
            return false;
        }
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC, operand1.dval / operand2.dval, false, "" });
    }
    else if (oper == "%")
    {
        if (operand2.dval == 0)
        {
            if (0)
                FPS_PRINT_ERROR("Division by 0", NULL);
            return false;
        }
        operands.push(
            Expression::Operand{ Expression::DataType::NUMERIC, fmod(operand1.dval, operand2.dval), false, "" });
    }
    else if (oper == "**")
        operands.push(
            Expression::Operand{ Expression::DataType::NUMERIC, pow(operand1.dval, operand2.dval), false, "" });

    // Bitwise operators
    else if (oper == "&")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC,
                                           (double)((int)operand1.dval & (int)operand2.dval), false, "" });
    else if (oper == "|")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC,
                                           (double)((int)operand1.dval | (int)operand2.dval), false, "" });
    else if (oper == "^")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC,
                                           (double)((int)operand1.dval ^ (int)operand2.dval), false, "" });
    else if (oper == "<<")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC,
                                           (double)((int)operand1.dval << (int)operand2.dval), false, "" });
    else if (oper == ">>")
        operands.push(Expression::Operand{ Expression::DataType::NUMERIC,
                                           (double)((int)operand1.dval >> (int)operand2.dval), false, "" });
    else
        FPS_PRINT_WARN("Operation {} not supported", oper);

    // Pop the operator from the operator stack and proceed to the next operator,
    // if possible
    operators.pop();

    return true;
}

/**
 * @brief Helper function for running a supported function and evaluating the
 * results
 *
 * @param operators the operators stack
 * @param operands the operands stack
 * @return true if the function runs successfully
 */
inline bool func(std::stack<Expression::Operator>& operators, std::stack<Expression::Operand>& operands)
{
    // If the operands/operators is empty, then we have an invalid
    // operands/operators stack
    if (operators.empty())
    {
        FPS_PRINT_DEBUG("The list of operators is empty. Skipping func", NULL);
        return false;
    }

    const std::string funcName = operators.top();
    if (funcName == "sqrt")
    {
        if (operands.empty())
        {
            FPS_PRINT_DEBUG(
                "The list of operands is empty. We need at least one "
                "operand to run sqrt func. Skipping func",
                NULL);
            return false;
        }

        Expression::Operand operand = operands.top();
        operands.pop();
        if (operand.type != Expression::DataType::NUMERIC)
        {
            FPS_PRINT_ERROR(
                "The operand type is not NUMERIC, which is needed for "
                "sqrt func. Skipping func",
                NULL);
            return false;
        }
        double result = Math::squareRoot(operand.dval);
        Expression::Operand resultOperand{ Expression::DataType::NUMERIC, result, false, "" };
        operands.push(resultOperand);
    }
    if (funcName == "max")
    {
        if (operands.size() < 2)
        {
            FPS_PRINT_DEBUG(
                "The list of operands is less than two. We need at least "
                "two operands to run max func. Skipping func",
                NULL);
            return false;
        }

        Expression::Operand operand2 = operands.top();
        operands.pop();
        Expression::Operand operand1 = operands.top();
        operands.pop();
        if (operand1.type != Expression::DataType::NUMERIC || operand2.type != Expression::DataType::NUMERIC)
        {
            FPS_PRINT_ERROR(
                "The operand(s) type is not NUMERIC, which is needed for "
                "max func. Skipping func",
                NULL);
            return false;
        }

        std::vector<double> args;
        args.reserve(2);
        args.push_back(operand1.dval);
        args.push_back(operand2.dval);
        double result = Math::max(args);
        Expression::Operand resultOperand{ Expression::DataType::NUMERIC, result, false, "" };
        operands.push(resultOperand);
    }
    if (funcName == "min")
    {
        if (operands.size() < 2)
        {
            FPS_PRINT_DEBUG(
                "The list of operands is less than two. We need two "
                "operands to run min func. Skipping func",
                NULL);
            return false;
        }

        Expression::Operand operand2 = operands.top();
        operands.pop();
        Expression::Operand operand1 = operands.top();
        operands.pop();
        if (operand1.type != Expression::DataType::NUMERIC || operand2.type != Expression::DataType::NUMERIC)
        {
            FPS_PRINT_ERROR(
                "The operand(s) type is not NUMERIC, which is needed for "
                "min func. Skipping func",
                NULL);
            return false;
        }

        std::vector<double> args;
        args.reserve(2);
        args.push_back(operand1.dval);
        args.push_back(operand2.dval);
        double result = Math::min(args);
        Expression::Operand resultOperand{ Expression::DataType::NUMERIC, result, false, "" };
        operands.push(resultOperand);
    }
    if (funcName == "if")
    {
        // We need three arguments to compose conditional if like so: if [boolean],
        // [expr_if_true], [expr_if_false]
        if (operands.size() < 3)
        {
            FPS_PRINT_DEBUG(
                "The list of operands is less than two. We need at three "
                "operands to run if statement. Skipping conditional "
                "evaluation",
                NULL);
            return false;
        }

        Expression::Operand valIfFalse = operands.top();
        operands.pop();
        Expression::Operand valIfTrue = operands.top();
        operands.pop();
        Expression::Operand conditionVal = operands.top();
        operands.pop();

        if (conditionVal.type == Expression::DataType::STRING)
        {
            FPS_PRINT_WARN(
                "The condition operand type is not NUMERIC, which is "
                "needed for if func. Skipping conditional evaluation",
                NULL);
            return false;
        }

        bool condition = false;
        // If the condition operand is a numeric type, convert stored value to a
        // boolean
        if (conditionVal.type == Expression::DataType::NUMERIC)
            condition = (bool)conditionVal.dval;

        else if (conditionVal.type == Expression::DataType::BOOL)
            condition = conditionVal.bval;

        condition ? operands.push(valIfTrue) : operands.push(valIfFalse);
    }

    // Pop the operator from the operator stack and proceed to the next operator,
    // if possible
    operators.pop();

    return true;
}

/**
 * @brief Helper function that checks if the expression is valid
 *
 * @param av the assetVar that contains the expression
 * @param exprParam the expression parameter name
 *
 * @return true if the expression is valid, meaning not null and does not
 * contain n/a
 */
inline bool validExpr(assetVar* av, const char* exprParam)
{
    if (!av->gotParam(exprParam) || !av->getcParam(exprParam) || strcmp(av->getcParam(exprParam), "n/a") == 0)
    {
        FPS_PRINT_DEBUG(
            "expression [{}] not defined for assetVar [{}:{}]",
            (av->gotParam(exprParam) && av->getcParam(exprParam)) ? av->getcParam(exprParam) : "Not Defined", av->comp,
            av->name);
        return false;
    }
    return true;
}
}  // namespace Expression

namespace CalculatorUtils
{
/**
 * @brief Adds the assetVar's value to the list of operands, if the data type of
 * the assetVar is supported
 *
 * @param type the assetVar's'data type
 * @param av the assetVar
 * @param operands the list of operands
 * @return true if the value is added to the list of operands or false otherwise
 */
inline bool addOperandForAv(int type, assetVar* av, std::vector<Expression::Operand>& operands)
{
    switch (type)
    {
        case assetVar::ATypes::AINT:
        {
            FPS_PRINT_DEBUG("assetVar {} is int type", av->name);
            Expression::Operand operand{ Expression::DataType::NUMERIC, (double)av->getiVal(), false, "" };
            operands.push_back(operand);
            break;
        }
        case assetVar::ATypes::AFLOAT:
        {
            FPS_PRINT_DEBUG("assetVar {} is float type", av->name);
            Expression::Operand operand{ Expression::DataType::NUMERIC, av->getdVal(), false, "" };
            operands.push_back(operand);
            break;
        }
        case assetVar::ATypes::ABOOL:
        {
            FPS_PRINT_DEBUG("assetVar {} is boolean type", av->name);
            Expression::Operand operand{};
            operand.type = Expression::DataType::BOOL;
            operand.bval = av->getbVal();
            operands.push_back(operand);
            break;
        }
        case assetVar::ATypes::ASTRING:
        {
            FPS_PRINT_DEBUG("assetVar {} is string type", av->name);
            Expression::Operand operand{};
            operand.type = Expression::DataType::STRING;
            operand.sval = av->getcVal() ? fmt::format("{}", av->getcVal()) : "no string val";
            operands.push_back(operand);
            break;
        }
        default:
            FPS_PRINT_ERROR(
                "Type not supported for assetVar {}. Supported types are: "
                "AINT, AFLOAT, ASTRING, and ABOOL",
                av->name);
            return false;
    }
    return true;
}

/**
 * @brief Adds the assetVar parameter's value to the list of operands, if the
 * data type of the parameter is supported
 *
 * @param type the assetVar parameter's data type
 * @param av the assetVar that contains the parameter
 * @param avParam the name of the parameter
 * @param operands the list of operands
 * @return true if the value is added to the list of operands or false otherwise
 */
inline bool addOperandForParam(int type, assetVar* av, const std::string avParam,
                               std::vector<Expression::Operand>& operands)
{
    switch (type)
    {
        case assFeat::AINT:
        {
            FPS_PRINT_DEBUG("param {} is int type", avParam);
            Expression::Operand operand{ Expression::DataType::NUMERIC, (double)av->getdParam(avParam.c_str()), false,
                                         "" };
            operands.push_back(operand);
            break;
        }
        case assFeat::AFLOAT:
        {
            FPS_PRINT_DEBUG("param {} is float type", avParam);
            Expression::Operand operand = { Expression::DataType::NUMERIC, av->getdParam(avParam.c_str()), false, "" };
            operands.push_back(operand);
            break;
        }
        case assFeat::ABOOL:
        {
            FPS_PRINT_DEBUG("param {} is boolean type", avParam);
            Expression::Operand operand{};
            operand.type = Expression::DataType::BOOL;
            operand.bval = av->getbParam(avParam.c_str());
            operands.push_back(operand);
            break;
        }
        case assFeat::ASTRING:
        {
            FPS_PRINT_DEBUG("param {} is string type", avParam);
            Expression::Operand operand{};
            operand.type = Expression::DataType::STRING;
            operand.sval = av->getcParam(avParam.c_str()) ? fmt::format("{}", av->getcParam(avParam.c_str()))
                                                          : "no string val";
            operands.push_back(operand);
            break;
        }
        default:
            FPS_PRINT_WARN("featType {} is not supported here for param {}", type, avParam);
            return false;
    }
    return true;
}

/**
 * @brief Helper function for getting a list of operands
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the asset var to get list of operands for
 * @param operands the list of operands
 * @return true if retrieval of operands is successful
 */
inline bool getOperands(varsmap& vmap, varmap& amap, assetVar* av, std::vector<Expression::Operand>& operands)
{
    VarMapUtils* vm = av->am->vm;

    // If includeCurrVal is true, add the assetVar's current value to the list of
    // operands
    if (av->gotParam("includeCurrVal") && av->getbParam("includeCurrVal"))
    {
        // Attempt to add the current assetVar's value to the list of operands
        if (!addOperandForAv(av->type, av, operands))
            return false;
    }

    // Add the assetVar parameters to the list of operands
    int numVars = av->getiParam("numVars");
    if (numVars > 0)
    {
        // Add the assetVar parameters to the list of operands
        for (int i = 1; i <= numVars; i++)
        {
            const std::string varParam = fmt::format("variable{}", i);
            const char* avParamName = av->getcParam(varParam.c_str());
            if (!avParamName)
            {
                FPS_PRINT_WARN("assetVar [{}:{}] does not have param {} or param value is null", av->comp, av->name,
                               varParam);
                return false;
            }

            const std::string avUri(avParamName);
            if (avUri.empty())
            {
                FPS_PRINT_WARN("assetVar [{}:{}] does not have parameter [{}]", av->comp, av->name, varParam);
                return false;
            }
            FPS_PRINT_DEBUG(
                "assetVar [{}:{}] has parameter [{}]. Attempting to "
                "adding to list of operands",
                av->comp, av->name, varParam);

            assetUri my(avUri.c_str());
            if (!my.Uri && !my.Var)
            {
                FPS_PRINT_WARN("assetVar [{}:{}] does not have uri [{}] and/or var [{}]", av->comp, av->name,
                               cstr{ my.Uri }, cstr{ my.Var });
                return false;
            }

            // If we have something like bms:RackCurrent, then this is an aggregation
            // for all asset managers or instances under the asset manager In this
            // case, we'll want to add all of the asset manager or instance's assetVar
            // value to the list of operands
            if (strcmp(my.Uri, av->am->name.c_str()) == 0)
            {
                const std::string avName = fmt::format("{}", my.Var);
                const std::string avParam = fmt::format("{}", my.Param ? my.Param : "");

                // If the asset manager map is not empty, we'll take the aggregation of
                // all the asset managers under the parent asset manager
                if (!av->am->assetManMap.empty())
                {
                    // Iterate through all asset managers in the asset map and see if the
                    // asset manager contains the assetVar targeted for aggregation
                    for (auto& pair : av->am->assetManMap)
                    {
                        FPS_PRINT_DEBUG("pair.first: {} pair.second: {} avName: {}", pair.first, pair.second->name,
                                        avName);
                        asset_manager* ami = pair.second;
                        assetVar* amiVar = ami->amap[avName];
                        if (!amiVar)
                        {
                            FPS_PRINT_DEBUG("amiVar with ami {} and avName {} is null", ami->name, avName);
                            const std::string auri = fmt::format(
                                "{}/{}:{}", av->comp.substr(0, av->comp.find_last_of("/")), ami->name, avName);
                            FPS_PRINT_DEBUG("auri: {} av->comp: {} ami->name: {} avName: {}", auri, av->comp, ami->name,
                                            avName);
                            amiVar = vm->getVar(vmap, auri.c_str(), nullptr);
                            if (!amiVar)
                            {
                                FPS_PRINT_WARN("amiVar is null, where ami is {} and avName is {}", ami->name, avName);
                                return false;
                            }
                            ami->amap[avName] = amiVar;
                        }

                        // If we have an assetVar param, we'll include that in the list of
                        // operands instead of the assetVar's value
                        if (!avParam.empty() && amiVar->gotParam(avParam.c_str()))
                        {
                            FPS_PRINT_DEBUG(
                                "We got an asset param {} for av {}. We'll "
                                "include that in the list of operands instead",
                                avParam, amiVar->name);
                            if (!amiVar->extras)
                            {
                                FPS_PRINT_WARN("assetExtras is null for av {}", amiVar->name);
                                return false;
                            }
                            assetFeatDict* avDict = amiVar->extras->baseDict;
                            if (!avDict)
                            {
                                FPS_PRINT_WARN("baseDict is null for av {}", amiVar->name);
                                return false;
                            }

                            // Attempt to add assetVar param's value to the list of operands
                            int featType = avDict->getFeatType(avParam.c_str());
                            if (!addOperandForParam(featType, amiVar, avParam, operands))
                                return false;
                        }
                        else
                        {
                            if (!addOperandForAv(amiVar->type, amiVar, operands))
                                return false;
                        }
                    }
                }
                else
                {
                    // Iterate through all asset instances in the asset map and see if the
                    // asset instance contains the assetVar targeted for aggregation
                    for (auto& pair : av->am->assetMap)
                    {
                        FPS_PRINT_DEBUG("amiVar with ami {} and avName {} is null", av->name, avName);
                        asset* ai = pair.second;
                        assetVar* aiVar = ai->amap[avName];
                        if (!aiVar)
                        {
                            FPS_PRINT_DEBUG("aiVar with ai {} and avName {} is null", ai->name, avName);
                            const std::string auri = fmt::format(
                                "{}/{}:{}", av->comp.substr(0, av->comp.find_last_of("/")), ai->name, avName);
                            FPS_PRINT_DEBUG("auri: {} av->comp: {} ai->name: {} avName: {}", auri, av->comp, ai->name,
                                            avName);
                            aiVar = vm->getVar(vmap, auri.c_str(), nullptr);
                            if (!aiVar)
                            {
                                FPS_PRINT_WARN("aiVar is null, where ai is {} and avName is {}", ai->name, avName);
                                return false;
                            }
                            ai->amap[avName] = aiVar;
                        }

                        // If we have an assetVar param, we'll include that in the list of
                        // operands instead of the assetVar's value
                        if (!avParam.empty() && aiVar->gotParam(avParam.c_str()))
                        {
                            FPS_PRINT_DEBUG(
                                "We got an asset param {} for av {}. We'll "
                                "include that in the list of operands instead",
                                avParam, aiVar->name);
                            if (!aiVar->extras)
                            {
                                FPS_PRINT_WARN("assetExtras is null for av {}", aiVar->name);
                                return false;
                            }
                            assetFeatDict* avDict = aiVar->extras->baseDict;
                            if (!avDict)
                            {
                                FPS_PRINT_WARN("baseDict is null for av {}", aiVar->name);
                                return false;
                            }

                            // Attempt to add assetVar param's value to the list of operands
                            int featType = avDict->getFeatType(avParam.c_str());
                            if (!addOperandForParam(featType, aiVar, avParam, operands))
                                return false;
                        }
                        else
                        {
                            if (!addOperandForAv(aiVar->type, aiVar, operands))
                                return false;
                        }
                    }
                }
            }
            // No aggregation. We'll try to add the assetVar's value to the list of
            // operands as normal
            else
            {
                // char* xxname = (char *)avUri.c_str();
                // avUri = /status/bms:BMSCurrent -> use my.Var
                // avUri = BMSCurrent -> use my.Uri
                assetVar* anotherAv;
                if (!my.Var)
                {
                    if (!amap[avUri])
                    {
                        amap[avUri] = vm->getVar(vmap, avUri.c_str(), nullptr);
                    }
                    anotherAv = amap[avUri];
                }
                else
                {
                    anotherAv = vm->getVar(vmap, avUri.c_str(), nullptr);
                }

                if (!anotherAv)
                {
                    FPS_PRINT_WARN("assetVar in parameter [{}] of assetVar [{}] does not exist", varParam, av->name);
                    return false;
                }

                FPS_PRINT_DEBUG(
                    "assetVar {} in parameter [{}] of assetVar [{}] "
                    "exists. Adding val to list of operands",
                    anotherAv->name, varParam, av->name);

                // If a param is defined in the assetVar uri, check if that param exists
                // and include that value to the list of operands instead
                const std::string avParam = fmt::format("{}", my.Param ? my.Param : "");
                if (!avParam.empty() && anotherAv->gotParam(avParam.c_str()))
                {
                    FPS_PRINT_DEBUG(
                        "We got an asset param {} for av {}. We'll include "
                        "that in the list of operands instead",
                        avParam, av->name);
                    if (!anotherAv->extras)
                    {
                        FPS_PRINT_WARN("assetExtras is null for av {}", av->name);
                        return false;
                    }
                    assetFeatDict* avDict = anotherAv->extras->baseDict;
                    if (!avDict)
                    {
                        FPS_PRINT_WARN("baseDict is null for av {}", av->name);
                        return false;
                    }

                    // Attempt to add assetVar param's value to the list of operands
                    int featType = avDict->getFeatType(avParam.c_str());
                    if (!addOperandForParam(featType, anotherAv, avParam, operands))
                        return false;
                }
                else
                {
                    // Attempt to add assetVar's value to the list of operands
                    if (!addOperandForAv(anotherAv->type, anotherAv, operands))
                        return false;
                }
            }
        }
    }
    return true;
}

/**
 * @brief Helper function for getting a list of operand info (ex.: uri, name)
 *
 * Ex.: If we have an assetVar with the following info:
 * /status/pcs:ActivePowerSetpoint, then the operand info we'll get is
 * /status/pcs:ActivePowerSetpoint
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the asset var to get list of operand info for
 * @param operandINfo the list of operand info
 * @return true if retrieval of operand info is successful
 */
inline bool getOperandInfo(varsmap& vmap, varmap& amap, assetVar* av, std::vector<std::string>& operandInfo)
{
    VarMapUtils* vm = av->am->vm;

    // If includeCurrVal is true, add the assetVar's info to the list of operand
    // info
    if (av->gotParam("includeCurrVal") && av->getbParam("includeCurrVal"))
        operandInfo.push_back(fmt::format("{}", av->getfName()));

    // Add the assetVar parameters' info to the list of operand info
    int numVars = av->getiParam("numVars");
    if (numVars > 0)
    {
        for (int i = 1; i <= numVars; i++)
        {
            const std::string varParam = fmt::format("variable{}", i);
            const char* avParamName = av->getcParam(varParam.c_str());
            if (!avParamName)
            {
                FPS_PRINT_WARN("assetVar [{}] does not have param {} or param value is null", av->name, varParam);
                return false;
            }

            const std::string avUri(avParamName);
            if (avUri.empty())
            {
                FPS_PRINT_WARN("assetVar [{}] does not have parameter [{}]", av->name, varParam);
                return false;
            }

            FPS_PRINT_INFO /*DEBUG*/ (
                "assetVar [{}] has parameter [{}]. Attempting "
                "to adding to list of operands",
                av->name, varParam);
            assetUri my(avUri.c_str());
            // Store the assetVar in the local data map
            assetVar* anotherAv;
            if (!my.Var)
            {
                if (!amap[avUri])
                {
                    amap[avUri] = vm->getVar(vmap, avUri.c_str(), nullptr);
                }
                anotherAv = amap[avUri];
            }
            else
            {
                anotherAv = vm->getVar(vmap, avUri.c_str(), nullptr);
            }
            if (!anotherAv)
            {
                FPS_PRINT_WARN("assetVar in parameter [{}] of assetVar [{}] does not exist", varParam, av->name);
                return false;
            }

            FPS_PRINT_INFO /*DEBUG*/ (
                "assetVar {} in parameter [{}] of assetVar "
                "[{}] exists. Adding info (uri, name, "
                "parameter) to list of operand info",
                anotherAv->name, varParam, av->name);
            operandInfo.push_back(fmt::format("{}", anotherAv->getfName()));
        }
    }
    return true;
}

/**
 * @brief Helper function for getting a list of operands of numeric type
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the assetVar to get list of operands of type double for
 * @param operandsDbl the list of operands to convert to double
 * @return true if retrieval of operands as doubles is successful
 */
inline bool getOperandsDbl(varsmap& vmap, varmap& amap, assetVar* av, std::vector<double>& operandsDbl)
{
    // Collection used to store list of operands
    std::vector<Expression::Operand> operands;

    // Check if we are unable to retrieve the list of operands
    if (!CalculatorUtils::getOperands(vmap, amap, av, operands))
    {
        FPS_PRINT_WARN("Unable to retrieve list of operands for [{}:{}]", av->comp, av->name);
        return false;
    }

    // Get the list of operands of type double
    for (const auto& operand : operands)
    {
        if (operand.type == Expression::DataType::STRING)
        {
            FPS_PRINT_DEBUG(
                "operand data type {} is a string, which we cannot add "
                "to the list of operands of type double",
                operand.type);
            return false;
        }
        if (operand.type == Expression::DataType::BOOL)
            operandsDbl.push_back((double)operand.bval);
        else
            operandsDbl.push_back(operand.dval);
    }
    return true;
}

/**
 * @brief Helper function for getting the value of the operand based on the
 * operand's data type
 *
 * @return the value of the operand or empty string if the given data type is
 * not supported
 */
inline const std::string getOperandVal(Expression::Operand operand)
{
    switch (operand.type)
    {
        case Expression::DataType::NUMERIC:
            FPS_PRINT_DEBUG("operand val {} is int type", operand.dval);
            return fmt::format("{}", operand.dval);
        case Expression::DataType::BOOL:
            FPS_PRINT_DEBUG("operand val {} is boolean type", operand.bval);
            return fmt::format("{}", operand.bval);
        case Expression::DataType::STRING:
            FPS_PRINT_DEBUG("operand val {} is string type", operand.sval);
            return fmt::format("{}", operand.sval);
        default:
            FPS_PRINT_ERROR(
                "Type {} not supported for operand. Supported expression "
                "data types are: NUMERIC, BOOL, and STRING",
                operand.type);
            return {};
    }
}

/**
 * @brief Helper function that replaces all occurrences of a substring with a
 * new substring
 *
 * @param str the string
 * @param from the substring to replace
 * @param to the substring to replace with
 */
inline void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
        return;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

/**
 * @brief Construct and return a string, which contains the expression and the
 * assetVar uri and name associated with the expression, if placeholders are
 * used
 *
 * Ex.: 10 + {1}, where {1} = /controls/pcs:ActivePowerSetpoint
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the assetVar that contains the expression
 * @param exprParam the expression parameter name
 *
 * @return the string containing the expression and associated assetVar info if
 * placeholders are present
 */
inline const std::string getExpressionInfo(varsmap& vmap, varmap& amap, assetVar* av, const char* exprParam)
{
    if (!Expression::validExpr(av, exprParam))
    {
        FPS_PRINT_DEBUG("Expression not valid for [{}:{}]", av->comp, av->name);
        return {};
    }

    const std::string expr(av->getcParam(exprParam));
    std::string exprInfo(av->getcParam(exprParam));

    // Collection used to store list of operands and operand info
    std::vector<Expression::Operand> operands;
    std::vector<std::string> operandInfo;

    // If we are unable to retrieve the list of operands, return empty string to
    // indicate invalid expression
    if (!getOperands(vmap, amap, av, operands) || !getOperandInfo(vmap, amap, av, operandInfo))
    {
        FPS_PRINT_WARN("Unable to retrieve list of operands and/or info for [{}:{}]", av->comp, av->name);
        return {};
    }

    // Look for placeholders, if any, and replace with operand info
    // Ex.: {1} + 1 + {2}, where {1} and {2} are placeholders
    size_t startIdx = expr.find("{");
    size_t endIdx = expr.find("}");
    std::vector<int> placeholders;
    while (startIdx != std::string::npos && endIdx != std::string::npos)
    {
        FPS_PRINT_DEBUG("Found { and }. startIdx {} endIdx {}. Looking for placeholder...", startIdx, endIdx);

        // Get the placeholder and the value within {}
        std::string placeholder = expr.substr(startIdx, (endIdx + 1 - startIdx));
        const std::string val = expr.substr(startIdx + 1, (endIdx - 1 - startIdx));
        FPS_PRINT_DEBUG("placeholder {} value {}", placeholder, val);

        // Convert placeholder value to int and check if that new index value is
        // within the valid index ranges in the operands vector
        int idx = std::stoi(val) - 1;
        if (idx < 0 || idx > (int)operandInfo.size() - 1 || idx > (int)operands.size() - 1)
        {
            FPS_PRINT_DEBUG(
                "placeholder value {} is not within the valid index "
                "range in the list of operands and/or operand info",
                idx);
            return {};
        }

        // Replace any occurrence of the placeholder with the operand info
        std::string info = fmt::format("[{}:{}]", operandInfo[idx], getOperandVal(operands[idx]));
        replaceAll(exprInfo, placeholder, info);

        startIdx = expr.find("{", startIdx + 1);
        endIdx = expr.find("}", endIdx + 1);
    }

    return exprInfo;
}

/**
 * @brief Parse the expression. Perform variable substitution if placeholders
 * are found in the expression and the corresponding operand can be found
 *
 * Ex.: expression = {1} + 2, where {1} represents operand no. 1
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the assetVar that contains the expression
 * @param exprParam the expression parameter name
 *
 * @return the expression if parsed successfully or an empty string otherwise
 */
inline const std::string parseExpr(varsmap& vmap, varmap& amap, assetVar* av, const char* exprParam)
{
    if (!Expression::validExpr(av, exprParam))
    {
        FPS_PRINT_DEBUG("Expression not valid for [{}:{}] ", av->comp, av->name);
        return {};
    }

    // Collection used to store list of operands
    std::vector<Expression::Operand> operands;

    // If we are unable to retrieve the list of operands, return empty string
    if (!getOperands(vmap, amap, av, operands))
    {
        FPS_PRINT_WARN("Unable to retrieve list of operands for [{}:{}]", av->comp, av->name);
        return {};
    }

    std::string expr(av->getcParam(exprParam));

    // First pass - Replace commas with whitespaces and add whitespace to
    // parenthesis in preparation for parsing
    if (expr.find(",") != std::string::npos)
    {
        replaceAll(expr, ",", " ");
        // av->setParam(exprParam, (char*)expr.c_str());
    }

    if (expr.find("(") != std::string::npos)
    {
        replaceAll(expr, "(", " ( ");
        // av->setParam(exprParam, (char*)expr.c_str());
    }

    if (expr.find(")") != std::string::npos)
    {
        replaceAll(expr, ")", " ) ");
        // av->setParam(exprParam, (char*)expr.c_str());
    }

    // Second pass - Replace placeholders with the values in the operands vector
    // in the expression The placeholder value is the index value in the operands
    // vector Ex.: {1} + 1 + {2} evaluates to 2 + 1 + 3, where {1} = 2 and {2} = 3
    size_t startIdx = expr.find("{");
    size_t endIdx = expr.find("}");
    while (startIdx != std::string::npos && endIdx != std::string::npos)
    {
        FPS_PRINT_DEBUG("Found { and }. startIdx {} endIdx {}. Looking for placeholder...", startIdx, endIdx);

        // Get the placeholder value within {}
        const std::string placeholder = expr.substr(startIdx + 1, (endIdx - 1 - startIdx));
        FPS_PRINT_DEBUG("placeholder {}", placeholder);

        // Check if the placeholder is a number. If so, this is a positional
        // placeholder Ex.: {1} represents the first element in the list of operands

        // Otherwise, check if the placeholder contains the key values
        // Ex.: value, variable1, variable2, etc.

        // Convert placeholder value to int and check if that new index value is
        // within the valid index ranges in the operands vector
        int idx = std::stoi(placeholder) - 1;
        if (idx < 0 || idx > (int)operands.size() - 1)
        {
            FPS_PRINT_DEBUG(
                "placeholder value {} is the valid index range in the "
                "list of operands",
                idx);
            return {};
        }

        // Get the value of the operand depending on the data type
        const std::string val = getOperandVal(operands[idx]);
        expr.replace(startIdx, endIdx + 1 - startIdx, val);
        startIdx = expr.find("{", startIdx);
        endIdx = expr.find("}", endIdx);
    }

    return expr;
}

/**
 * @brief Helper function for evaluating math expression using
 * the shunting-yard algorithm
 * (https://en.wikipedia.org/wiki/Shunting-yard_algorithm)
 *
 * @param expr the math expression to evaluate
 * @return the results
 */
inline Expression::Result evaluateExpr(const std::string& expr)
{
    FPS_PRINT_DEBUG("Evaluating expression {}", expr);
    std::string token;
    std::stringstream ss(expr);  // Insert the expression into string stream

    std::stack<Expression::Operator> operators;  // stack for operators
    std::stack<Expression::Operand> operands;    // stack for operands

    // Tokenize the expression by whitespace
    while (ss >> token)
    {
        if (Expression::isNumber(token))
        {
            FPS_PRINT_DEBUG("Found number {}. Adding to operand stack", token);
            Expression::Operand operand{ Expression::DataType::NUMERIC, stod(token), false, "" };
            operands.push(operand);
        }
        else if (Expression::isBoolean(token))
        {
            FPS_PRINT_DEBUG("Found boolean {}. Adding to operand stack", token);
            Expression::Operand operand{};
            operand.type = Expression::DataType::BOOL;
            operand.bval = (token == "true") ? true : false;
            operands.push(operand);
        }
        else if (Expression::isOperator(token))
        {
            // If operator at the top of the stack has greater precedence or the
            // operator at the top of the stack has equal precendence and token is
            // left associative and the operator is not a (, pop operators from the
            // operator stack and perform calculation on the two operands in the
            // operands stack
            while (!operators.empty() &&
                   (Expression::getPrecedence(operators.top()) > Expression::getPrecedence(token) ||
                    (Expression::getPrecedence(operators.top()) == Expression::getPrecedence(token) &&
                     Expression::operatorMap[token].assoc == Expression::Associativity::leftAssociative)) &&
                   operators.top() != "(")
            {
                if (!Expression::evaluate(operators, operands))
                {
                    FPS_PRINT_WARN("Unsuccessful evaluation of expression {}", expr);
                    return Expression::Result{ Expression::DataType::NUMERIC, 0, false, "" };
                }
            }
            FPS_PRINT_DEBUG("Pushing operator {} to operator stack", token);
            operators.push(token);
        }
        // If the token is a function, push to operator stack
        else if (Expression::isFunction(token))
        {
            FPS_PRINT_DEBUG("Found function {}. Adding to operator stack", token);
            operators.push(token);
        }
        else if (token == "(")
        {
            FPS_PRINT_DEBUG("Pushing ( to operator stack", NULL);
            operators.push(token);
        }
        else if (token == ")")
        {
            FPS_PRINT_DEBUG("Found ) token", NULL);
            while (operators.top() != "(")
            {
                if (!Expression::evaluate(operators, operands))
                {
                    FPS_PRINT_WARN("Unsuccessful evaluation of expression {}", expr);
                    return Expression::Result{ Expression::DataType::NUMERIC, 0, false, "" };
                }
            }
            if (operators.top() == "(")
            {
                FPS_PRINT_DEBUG("Popping off ( from the operator stack", NULL);
                operators.pop();
            }
            // If there is a function token on top of the operator stack, pop the
            // function out and run function
            if (!operators.empty() && Expression::isFunction(operators.top()))
            {
                if (!Expression::func(operators, operands))
                {
                    FPS_PRINT_WARN("Unsuccessful evaluation of expression {} containing func", expr);
                    return Expression::Result{ Expression::DataType::NUMERIC, 0, false, "" };
                }
            }
        }
        else
        {
            FPS_PRINT_DEBUG("Found string {}. Adding to operand stack", token);
            Expression::Operand operand{};
            operand.type = Expression::DataType::STRING;
            operand.sval = token;
            operands.push(operand);
        }
    }

    // There should be no more tokens to read. At this stage, results should also
    // be calculated. In that case, return the result
    while (!operators.empty())
    {
        FPS_PRINT_DEBUG("About to evaluate operands with size {} operators with size {}", operands.size(),
                        operators.size());
        if (!Expression::evaluate(operators, operands))
        {
            FPS_PRINT_WARN("Unsuccessful evaluation of expression {}", expr);
            return Expression::Result{ Expression::DataType::NUMERIC, 0, false, "" };
        }
    }

    // If the operands stack is not empty, we may have an invalid expression. In
    // that case, exit out of function without reporting results
    if (operands.size() != 1)
    {
        FPS_PRINT_WARN(
            "The final size of the operands stack [{}] != 1, so "
            "expression {} is invalid",
            operands.size(), expr);
        return Expression::Result{ Expression::DataType::NUMERIC, 0, false, "" };
    }

    switch (operands.top().type)
    {
        case Expression::DataType::NUMERIC:
            return Expression::Result{ Expression::DataType::NUMERIC, operands.top().dval, false, "" };
        case Expression::DataType::BOOL:
            return Expression::Result{ Expression::DataType::BOOL, 0, operands.top().bval, "" };
        case Expression::DataType::STRING:
            return Expression::Result{ Expression::DataType::STRING, 0, false, operands.top().sval };
        default:
            FPS_PRINT_ERROR(
                "Type {} not supported for result. Supported expression "
                "data types are: NUMERIC, BOOL, and STRING",
                operands.top().type);
            return Expression::Result{ Expression::DataType::NUMERIC, 0, false, "" };
    }
}
}  // namespace CalculatorUtils
#endif