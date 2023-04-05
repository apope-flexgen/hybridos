import { Body, Controller, Get, Param, Post, Req } from '@nestjs/common'
import { ApiTags } from '@nestjs/swagger'
import { VariableOverrideService } from './ercotOverride.service'
import { VariableParams } from './params/variableNames.params';
import { EditOverrideParams } from './params/editOverride.params';
import { Request } from 'express';
import { FimsMsg } from 'src/fims/interfaces/fims.interface';

@ApiTags('variable-override')
@Controller('variable-override')
export class VariableOverrideController {
    constructor(private readonly variableOverrideService: VariableOverrideService) {}
    @Get('sites')
    async siteNames(): Promise<string | Record<string, unknown>> {
        return await this.variableOverrideService.getSiteNames()
    }
    @Get('variable-names')
    async variableNames(): Promise<string | Record<string, unknown>> {
        return await this.variableOverrideService.getVariableNames()
    }
    @Get('variable-values/:siteId')
    async variableValues(@Param() siteId: VariableParams): Promise<string | Record<string, unknown>> {
        return await this.variableOverrideService.getVariableValues(siteId)
    }
    @Post('override-value/:siteId/:variableName')
    async editOverride(@Req() request: Request, @Param() params: EditOverrideParams, @Body() newValue: {data: number | boolean}): Promise<{ data: string }>  {
        return await this.variableOverrideService.setOverrideValue(request, params, newValue)
    }
}
