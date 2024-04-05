#ifndef CALCULATEVAR_CPP
#define CALCULATEVAR_CPP

#include <algorithm>
#include <regex>
#include <stack>

#include "asset.h"
#include "calculator.hpp"
#include "ess_utils.hpp"
#include "formatters.hpp"

extern "C++" {
int CalculateVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

namespace CalculateVarUtils
{
using Operator = std::string;
// The list of all operators supported for CalculateVar
static std::vector<Operator> operators = { "valChangedAny", "valChangedAll" };

/**
 * @brief Sets the calculated value of type boolean to the assetVar. Converts
 * boolean to double if the assetVar's data type is numeric
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av the assetVar to set the results
 * @param result the calculated value of type boolean
 */
void setBoolVal(varsmap& vmap, assetVar* av, bool result)
{
    VarMapUtils* vm = av->am->vm;
    if (av->type == assetVar::ATypes::AINT || av->type == assetVar::ATypes::AFLOAT)
    {
        FPS_PRINT_DEBUG("av [{}] data type is numeric", av->getfName());
        double val = (double)result;
        if (val != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), val);
    }
    else
    {
        FPS_PRINT_DEBUG("av [{}] data type is boolean", av->getfName());
        if (result != av->getbVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), result);
    }
}
}  // namespace CalculateVarUtils

/**
 * @brief Initializes the parameters to be used for calculation function
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to initialize parameters for
 */
void setupCalculateVarParams(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    if (!av->am)
    {
        FPS_PRINT_DEBUG("asset manager for [{}:{}] is null", av->comp, av->name);
        return;
    }
    FPS_PRINT_INFO("Setting up params for [{}:{}]", av->comp, av->name);

    VarMapUtils* vm = av->am->vm;
    // Initialize the assetVar operand parameters to be used for calculation if
    // these do not exist
    if (!av->gotParam("numVars"))
        av->setParam("numVars", 0);

    for (int i = 1; i <= av->getiParam("numVars"); i++)
    {
        const std::string varParam = fmt::format("variable{}", i);
        if (!av->gotParam(varParam.c_str()) || !av->getcParam(varParam.c_str()))
        {
            FPS_PRINT_WARN("assetVar [{}] does not have param {} or param value is null", av->name, varParam);
            break;
        }
        const std::string avUri(av->getcParam(varParam.c_str()));
        FPS_PRINT_DEBUG("avUri {}  varParam {}   av {}", avUri, varParam, av->name);
        if (!avUri.empty())
        {
            assetUri my(avUri.c_str());

            // See if the assetVar exists in the data map if the uri is provided like
            // so: var_name
            if (!my.Var)
            {
                if (av->gotParam(varParam.c_str()) && av->getcParam(varParam.c_str()) && !amap[avUri])
                {
                    double dval = 0.0;
                    FPS_PRINT_WARN("Creating assetVar [{}] for [{}] in [{}]", avUri, varParam, av->getfName());
                    amap[avUri] = vm->setLinkVal(vmap, aname, "/status", avUri.c_str(), dval);
                    FPS_PRINT_WARN("Checking assetVar [{}]  fname [{}] am ptr [{}]", avUri, amap[avUri]->getfName(),
                                   fmt::ptr(amap[avUri]->am));
                }
            }
            else
            {
                // If we have the name of the asset and the name of the variable (ex.:
                // bms:RackCurrent), this indicates we are doing an aggregation In this
                // case, check to make sure all asset instance's assetVars exist
                if (strcmp(aname, my.Uri) == 0 && strcmp(av->am->name.c_str(), my.Uri) == 0)
                {
                    const std::string avName = fmt::format("{}", my.Var);
                    FPS_PRINT_INFO(
                        "Found the name of the asset [{}] in param [{}] in "
                        "[{}:{}]. This indicates we are including all of "
                        "asset instance's assetVars with name {} for "
                        "aggregation",
                        aname, varParam, av->comp, av->name, avName);

                    if (!av->am->assetManMap.empty())
                    {
                        // Iterate through all asset instances in the asset map and see if
                        // the asset instance contains the assetVar targeted for aggregation
                        for (auto& pair : av->am->assetManMap)
                        {
                            asset_manager* ami = pair.second;
                            FPS_PRINT_INFO("[{}:{}] parent asset manager [{}] asset manager [{}]", av->comp, av->name,
                                           aname, ami->name);
                            if (!ami->amap[avName])
                            {
                                double dval = 0.0;
                                FPS_PRINT_INFO("Creating AM, default double assetVar [{}] for [{}]", avName, ami->name);
                                ami->amap[avName] = vm->setLinkVal(vmap, ami->name.c_str(), "/status", avName.c_str(),
                                                                   dval);
                            }
                        }
                    }
                    else
                    {
                        // Iterate through all asset instances in the asset map and see if
                        // the asset instance contains the assetVar targeted for aggregation
                        for (auto& pair : av->am->assetMap)
                        {
                            asset* ai = pair.second;
                            FPS_PRINT_INFO("[{}:{}] asset manager [{}] asset instance [{}]", av->comp, av->name, avName,
                                           ai->name);
                            if (!ai->amap[avName])
                            {
                                double dval = 0.0;
                                FPS_PRINT_INFO("Creating AI default double assetVar [{}] for [{}]", avName, ai->name);
                                ai->amap[avName] = vm->setLinkVal(vmap, ai->name.c_str(), "/status", avName.c_str(),
                                                                  dval);
                                FPS_PRINT_INFO(
                                    "Checking AI assetVar [{}]  fname [{}] ptr am "
                                    "[{}] ai [{}] ",
                                    avName, ai->amap[avName]->getfName(), fmt::ptr(ai->amap[avName]->am),
                                    fmt::ptr(ai->amap[avName]->ai));
                            }
                        }
                    }
                }
                // We have a uri and a variable name. Something like this:
                // /component/asset_name:var_name
                else
                {
                    assetVar* avParam = vm->getVar(vmap, avUri.c_str(), nullptr);
                    // If the assetVar does not exist, create a new one. This should be
                    // added in vmap
                    if (!avParam)
                    {
                        // TODO give it a manager
                        FPS_PRINT_INFO(
                            "assetVar param {} does not exist. Creating new one "
                            "for am [{}]",
                            avUri, av->am->name);
                        vm->makeVar(vmap, avUri.c_str(), nullptr, 0);
                    }
                }
            }
        }
    }

    // Set the option to include current value in assetVar to false by default
    if (!av->gotParam("includeCurrVal"))
        av->setParam("includeCurrVal", false);

    // Initialize operation and expression parameters if these do not exist
    if (!av->gotParam("operation"))
        av->setParam("operation", (char*)"n/a");

    if (!av->gotParam("expression"))
        av->setParam("expression", (char*)"n/a");

    // Flag to check whether we want to use the expression or not
    // Note: if true, only the expression is evaluated. Otherwise, only the
    // operation is used
    if (!av->gotParam("useExpr"))
        av->setParam("useExpr", false);

    if (!av->gotParam("scale"))
        av->setParam("scale", 1);
}

/**
 * @brief Checks the math operator (+, -, *, /, **) and perform the operation
 * using the list of operands if valid. Results are then stored in assetVar
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the assetVar to store the results
 */
void checkExpression(varsmap& vmap, varmap& amap, assetVar* av, bool debug)
{
    // Parse the expression, making sure the expression is valid and replacing any
    // placeholders (ex.: {1}) with the appropriate corresponding operands
    // const PSW v1.1.0 possible leak ??
    const std::string& expr = CalculatorUtils::parseExpr(vmap, amap, av, "expression");
    if (expr.empty())
    {
        FPS_PRINT_WARN(
            "Unable to parse expression [{}] in [{}:{}]. Skipping "
            "expression evaluation",
            cstr{ av->getcParam("expression") }, av->comp, av->name);
        return;
    }

    if (debug)
        FPS_PRINT_INFO("expr char* [{}] expr string [{}]", av->getcParam("expression"), expr);

    // Evaluate expression using shunting-yard algorithm
    Expression::Result result = CalculatorUtils::evaluateExpr(expr);
    switch (result.type)
    {
        case Expression::DataType::NUMERIC:
        {
            if (debug)
                FPS_PRINT_INFO("{} = {}. Set to {}", expr, result.dval, av->name);
            if (result.dval != av->getdVal())
                av->am->vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), result.dval);

            break;
        }
        case Expression::DataType::BOOL:
        {
            if (debug)
                FPS_PRINT_INFO("{} = {}. Set to {}", expr, result.bval, av->name);

            // If the av type is an int, then we'll store the resulting boolean value as
            // an int
            CalculateVarUtils::setBoolVal(vmap, av, result.bval);
            break;
        }
        case Expression::DataType::STRING:
        {
            if (debug)
                FPS_PRINT_INFO("{} = {}. Set to {}", expr, result.sval, av->name);
            const char* resultVal = result.sval.c_str();
            if (!av->getcVal() || strcmp(resultVal, av->getcVal()) != 0)
                av->am->vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), resultVal);

            break;
        }
        default:
            FPS_PRINT_ERROR(
                "Type {} not supported for result. Supported expression "
                "data types are: NUMERIC, BOOL, and STRING",
                result.type);
    }
}

/**
 * @brief Parses the math expression (+, -, *, /, avg, etc.) and evaluates the
 * expression if valid. Results are then stored in assetVar
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the assetVar to store the results
 */
void checkOperation(varsmap& vmap, varmap& amap, assetVar* av)
{
    // Check if the operation is defined and if it is an operation that can be
    // used
    std::string oper(av->getcParam("operation"));
    if (oper.empty() || (std::find(Math::operators.begin(), Math::operators.end(), oper) == Math::operators.end() &&
                         std::find(CalculateVarUtils::operators.begin(), CalculateVarUtils::operators.end(), oper) ==
                             CalculateVarUtils::operators.end()))
    {
        // TODO after MVP send undefined action to log as well
        FPS_PRINT_WARN(
            "operation [{}] not defined for assetVar [{}:{}]. Supported "
            "operations are:\n"
            "    Addition           (+)\n"
            "    Subtraction        (-)\n"
            "    Multiplication     (*)\n"
            "    Division           (/)\n"
            "    Modulus Division   (%)\n"
            "    Average            (avg)\n"
            "    Percentage of      (pctOf)\n"
            "    Maximum            (max)\n"
            "    Minimum            (min)\n"
            "    Square Root        (sqrt)\n"
            "    Scale              (scale)\n"
            "    And                (and)\n"
            "    Or                 (or)\n"
            "    Greater than       (>)\n"
            "    Less than          (<)\n"
            "    Value Changed Any  (valChangedAny)\n"
            "    Value Changed All  (valChangedAll)\n",
            !oper.empty() ? oper : "no operator", av->comp, av->name);
        return;
    }
    VarMapUtils* vm = av->am->vm;

    // Check if we have operations that depend on mixed operand types
    if (std::find(CalculateVarUtils::operators.begin(), CalculateVarUtils::operators.end(), oper) !=
        CalculateVarUtils::operators.end())
    {
        // The list of operands of mixed types
        std::vector<assetVar*> operands;

        // If we are unable to retrieve the list of operands, skip calculations
        if (!ESSUtils::getAvList(vmap, amap, av, operands))
        {
            FPS_PRINT_ERROR(
                "Unable to retrieve list of assetVars as operands for "
                "[{}:{}]. Skipping calculations",
                av->comp, av->name);
            return;
        }

        // Run valChanged function for each operands here
        if (oper == "valChangedAny")
        {
            // Run valChanged function for all assetVar operands. Run valChangedReset
            // here since we are only interested in seeing at least one operand value
            // change
            bool results = std::any_of(operands.begin(), operands.end(),
                                       [](assetVar* av) { return av->valueChanged(); });
            if (results != av->getbVal())
            {
                // Only set value through vm and trigger actions when the resulting
                // value is true, which indicates all operand values have changed
                if (results)
                    vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);

                // Run valChangedReset afterwards to reset the changd flag for all
                // assetVars
                std::all_of(operands.begin(), operands.end(), [](assetVar* av) { return av->valueChangedReset(); });
                av->setVal(results);
            }

            FPS_PRINT_DEBUG("Ran valChangedAny operation for [{}:{}]. results {}", av->comp, av->name, results);
        }
        else if (oper == "valChangedAll")
        {
            // Run valChanged function for all assetVar operands. We do not want to
            // run valChangedReset here until all operand values have changed
            bool results = std::all_of(operands.begin(), operands.end(),
                                       [](assetVar* av) { return av->valueChanged(); });
            if (results != av->getbVal())
            {
                // Only set value through vm and trigger actions when the resulting
                // value is true, which indicates all operand values have changed
                if (results)
                    vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);

                // Run valChangedReset afterwards to reset the changd flag for all
                // assetVars
                std::all_of(operands.begin(), operands.end(), [](assetVar* av) { return av->valueChangedReset(); });
                av->setVal(results);
            }

            FPS_PRINT_DEBUG("Ran valChangedAll operation for [{}:{}]. results {}", av->comp, av->name, results);
        }

        return;
    }

    // Grab the list of operands as type double. Only valid if all of the operands
    // in the list are type double
    std::vector<double> operandsDbl;
    if (!CalculatorUtils::getOperandsDbl(vmap, amap, av, operandsDbl))
    {
        FPS_PRINT_DEBUG(
            "Unable to retrieve list of operands of type double for "
            "[{}:{}]. Skipping calculations",
            av->comp, av->name);
        return;
    }

    if (oper == "+")
    {
        FPS_PRINT_DEBUG("Perform addition for assetVar [{}]", av->name);
        double results = Math::add(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "-")
    {
        FPS_PRINT_DEBUG("Perform subtraction for assetVar [{}]", av->name);
        double results = Math::subtract(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "*")
    {
        FPS_PRINT_DEBUG("Perform multiplication for assetVar [{}]", av->name);
        double results = Math::multiply(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "/")
    {
        FPS_PRINT_DEBUG("Perform division for assetVar [{}]", av->name);
        double results = Math::divide(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "%")
    {
        FPS_PRINT_DEBUG("Perform modulus division for assetVar [{}]\n", av->name);
        double results = Math::modulus(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "avg")
    {
        FPS_PRINT_DEBUG("Perform average for assetVar [{}]", av->name);
        double results = Math::average(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "pctOf")
    {
        FPS_PRINT_DEBUG("Perform percentage of operation for assetVar [{}]", av->name);
        double results = Math::percentageOf(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "max")
    {
        FPS_PRINT_DEBUG("Perform max operation for assetVar [{}]", av->name);
        double results = Math::max(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "min")
    {
        FPS_PRINT_DEBUG("Perform min operation for assetVar [{}]", av->name);
        double results = Math::min(operandsDbl);
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "sqrt")
    {
        FPS_PRINT_DEBUG("Perform square root operation for assetVar [{}] with val [{:2.3f}]", av->name, av->getdVal());
        double results = Math::squareRoot(av->getdVal());
        if (av->gotParam("scale"))
            results = Math::scale(results, av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "scale")
    {
        FPS_PRINT_DEBUG("Perform scale operation for assetVar [{}] with val [{:2.3f}]", av->name, av->getdVal());
        double results = Math::scale(av->getdVal(), av->getdParam("scale"));
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == "and")
    {
        FPS_PRINT_DEBUG("Perform and operation for assetVar [{}]", av->name);
        bool results = std::all_of(operandsDbl.begin(), operandsDbl.end(), [](double val) { return val == 1; });

        // If the av type is an int, then we'll store the resulting boolean value as
        // an int
        CalculateVarUtils::setBoolVal(vmap, av, results);
    }
    else if (oper == "or")
    {
        FPS_PRINT_DEBUG("Perform or operation for assetVar [{}]", av->name);
        bool results = std::any_of(operandsDbl.begin(), operandsDbl.end(), [](double val) { return val == 1; });

        // If the av type is an int, then we'll store the resulting boolean value as
        // an int
        CalculateVarUtils::setBoolVal(vmap, av, results);
    }
    else if (oper == "stddev")
    {
        FPS_PRINT_DEBUG("Perform standard deviation operation for assetVar [{}]", av->name);
        double results = Math::stddev(operandsDbl);
        if (results != av->getdVal())
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), results);
    }
    else if (oper == ">")
    {
        if (operandsDbl.size() != 2)
        {
            FPS_PRINT_DEBUG("2 arguments needed for > for assetVar [{}]", av->name);
            return;
        }
        FPS_PRINT_DEBUG("Perform greater than operation for assetVar [{}]", av->name);
        bool results = operandsDbl[1] > operandsDbl[0];

        // If the av type is an int, then we'll store the resulting boolean value as
        // an int
        CalculateVarUtils::setBoolVal(vmap, av, results);
    }
    else if (oper == "<")
    {
        if (operandsDbl.size() != 2)
        {
            FPS_PRINT_DEBUG("2 arguments needed for < for assetVar [{}]", av->name);
            return;
        }
        FPS_PRINT_DEBUG("Perform less than operation for assetVar [{}]", av->name);
        bool results = operandsDbl[1] < operandsDbl[0];

        // If the av type is an int, then we'll store the resulting boolean value as
        // an int
        CalculateVarUtils::setBoolVal(vmap, av, results);
    }
}

/**
 * @brief Calculate the results using either the math operation or the math
 * expression
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param av the asset var to calculate results for
 */
bool checkEnable(VarMapUtils* vm, varsmap& vmap, assetVar* av, bool debug);

int CalculateVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(p_fims);
    if (!av)
    {
        FPS_PRINT_INFO("assetVar is null. Exiting function", NULL);
        return 0;
    }
    FPS_PRINT_DEBUG("av [{}] av->am [{}] aname [{}]", av->getfName(), av->am ? av->am->name : "null", cstr{ aname });
    VarMapUtils* vm = av->am->vm;
    bool debug = false;
    if (av->gotParam("debug"))
        debug = av->getbParam("debug");

    bool enabled = checkEnable(vm, vmap, av, debug);
    // essPerf ePerf(av->am, aname, __func__);
    std::string reloadStr = "CalculateVar_" + av->name;
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if (reload < 2)
    {
        FPS_PRINT_DEBUG("reload first run {}", aname, reload);
        setupCalculateVarParams(vmap, amap, aname, av);

        if (reload < 1)
        {
            FPS_PRINT_DEBUG("Setting assetVar [{}] for {} to /status", av->name, aname);
            amap[av->name] = vm->setLinkVal(vmap, aname, "/status", av->name.c_str(), av);
        }
        reload = 2;
        amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), reload);
        amap[reloadStr]->setVal(reload);
    }

    if (enabled)
    {
        // Perform calculation, depending on whether we got an expression or an
        // operation
        if (av->gotParam("useExpr") && av->getbParam("useExpr"))
            checkExpression(vmap, amap, av, debug);
        else
            checkOperation(vmap, amap, av);
    }
    return 0;
}

#endif