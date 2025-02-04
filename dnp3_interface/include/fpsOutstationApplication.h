/*
 * fpsOutstationApplication.h
 * author : pwilshire
 *    11 May, 2020
 *  looks like nothing is happening here ... yet 
 */

#ifndef NEW_OUTSTATIONAPPLICATION_H
#define NEW_OUTSTATIONAPPLICATION_H

#include <opendnp3/util/UTCTimestamp.h>

#include "opendnp3/app/Indexed.h"
#include "opendnp3/app/MeasurementTypes.h"
#include "opendnp3/app/parsing/ICollection.h"
#include "opendnp3/gen/AssignClassType.h"
#include "opendnp3/gen/PointClass.h"
#include "opendnp3/gen/RestartMode.h"
#include "opendnp3/link/ILinkListener.h"
#include "opendnp3/outstation/ApplicationIIN.h"
#include "opendnp3/outstation/IOutstationApplication.h"


#include <memory>
#include "dnp3_utils.h"
#include <iostream>


class fpsOutstationApplication final : public opendnp3::IOutstationApplication
{
public:
    fpsOutstationApplication(sysCfg* fpsDB) {sysdb=fpsDB;};

    static std::shared_ptr<fpsOutstationApplication> Create(sysCfg* fpsDB)
    {
        return std::make_shared<fpsOutstationApplication>(fpsDB);
    }

    bool SupportsWriteAbsoluteTime()
    {
        std::cout << " fpsOutstationApplication -<"<< __FUNCTION__<<" called \n";
        return false;
    }
    sysCfg* sysdb;

};
#endif
