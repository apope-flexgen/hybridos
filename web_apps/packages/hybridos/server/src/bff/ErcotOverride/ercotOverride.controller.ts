import { Body, Controller, Get, Param, Post, Req } from '@nestjs/common'
import { ApiDefaultResponse, ApiOkResponse, ApiSecurity, ApiTags } from '@nestjs/swagger'
import { ErcotOverrideService } from './ercotOverride.service'
import { Site, Variable } from './params/override.params';
import { Request } from 'express';
import { EditVariableRepnse } from './responses/editVariable.response';
import { NewValue } from './dto/newValue.dto';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { ErcotOverrideDescriptions } from './ercotOverride.constants';

@ApiTags('ercot-override')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({type: DefaultApiError})
@Controller('ercot-override')
export class ErcotOverrideController {
    constructor(private readonly ercotOverrideService: ErcotOverrideService) {}
    @ApiOkResponse({description: ErcotOverrideDescriptions.getResponse})
    @Get('sites')
    async siteNames(): Promise<string | Record<string, unknown>> {
        return await this.ercotOverrideService.getSiteNames()
    }
    @ApiOkResponse({description: ErcotOverrideDescriptions.getResponse})
    @Get('variable-names')
    async variableNames(): Promise<string | Record<string, unknown>> {
        return await this.ercotOverrideService.getVariableNames()
    }
    @Get('variable-values/:id')
    @ApiOkResponse({description: ErcotOverrideDescriptions.getResponse})
    async variableValues(
        @Param() id: Site
    ): Promise<string | Record<string, unknown>> {
        return await this.ercotOverrideService.getVariableValues(id)
    }
    @ApiOkResponse({description: ErcotOverrideDescriptions.setResponse})
    @Post('override-value/:id/:name')
    async editOverride(
        @Req() request: Request,
        @Param() id: Site,
        @Param() name: Variable,
        @Body() newValue: NewValue
    ): Promise<EditVariableRepnse>  {
        return await this.ercotOverrideService.setOverrideValue(request, id, name, newValue)
    }
}
