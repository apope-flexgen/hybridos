import { ApiProperty } from '@nestjs/swagger'
import { IsNotEmpty, IsString } from 'class-validator'
import { SchedulerDescriptions } from '../scheduler.constants'

export class ModeId {
    @ApiProperty({ description: SchedulerDescriptions.mode.modeId })
    @IsNotEmpty()
    @IsString()
    modeId: string
}
