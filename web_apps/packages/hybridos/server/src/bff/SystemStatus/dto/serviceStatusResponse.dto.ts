import { ApiProperty } from '@nestjs/swagger'
import {
    IsNumber,
    IsString,
    IsArray,
    IsOptional,
    IsBoolean,
} from 'class-validator'
import { systemStatusDescriptions } from '../systemStatus.constants'
import { ConnectionStatus } from '../../../../../shared/types/dtos/systemStatus.dto'

// TODO: right now, all fields are optional since we aren't getting all
// from backend. Update to remove optional validators once all data is flowing
export class ServiceStatusResponse {
    @ApiProperty({ 
        description: systemStatusDescriptions.dependencies 
    })
    @IsArray()
    @IsOptional()
    dependencies?: string[]

    @ApiProperty({
        description: systemStatusDescriptions.serviceName,
    })
    @IsString()
    @IsOptional()
    serviceName?: string

    @ApiProperty({
        description: systemStatusDescriptions.serviceStatus,
    })
    @IsString()
    @IsOptional()
    serviceStatus?: string

    @ApiProperty({
        description: systemStatusDescriptions.connectionStatus,
    })
    @IsOptional()
    connectionStatus?: ConnectionStatus

    @ApiProperty({
        description: systemStatusDescriptions.cpuUsage,
    })
    @IsNumber()
    @IsOptional()
    cpuUsage?: number

    @ApiProperty({
        description: systemStatusDescriptions.memoryUsage,
    })
    @IsNumber()
    @IsOptional()
    memoryUsage?: number

    @ApiProperty({
        description: systemStatusDescriptions.serviceUptime,
    })
    @IsNumber()
    @IsOptional()
    uptime?: number
    
    @ApiProperty({
        description: systemStatusDescriptions.lastRestart,
    })
    @IsString()
    @IsOptional()
    lastRestart?: string

    @ApiProperty({
        description: systemStatusDescriptions.actions,
    })
    @IsArray()
    @IsOptional()
    actions?: ServiceAction[]
}

export class ServiceAction {
    @ApiProperty({ 
        description: systemStatusDescriptions.actionEnabled 
    })
    @IsBoolean()
    @IsOptional()
    enabled?: boolean

    @ApiProperty({
        description: systemStatusDescriptions.actionTitle,
    })
    @IsString()
    @IsOptional()
    action?: 'start' | 'stop' | 'restart'
}
