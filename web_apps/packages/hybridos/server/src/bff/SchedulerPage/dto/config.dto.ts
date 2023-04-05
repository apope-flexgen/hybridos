import { IsOptional, IsString } from 'class-validator'
import { Configuration } from '../../../../../shared/types/dtos/scheduler.dto';

export class SchedulerConfiguration {
    @IsString()
        scheduler_type: Configuration['scheduler_type']
    @IsOptional()
        local_schedule?: Configuration['local_schedule']
    @IsOptional()
        web_sockets?: Configuration['web_sockets']
    @IsOptional()
        scada?: Configuration['scada']

}