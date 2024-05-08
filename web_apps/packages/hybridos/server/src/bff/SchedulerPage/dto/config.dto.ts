import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsBoolean, IsNumber, IsObject, IsOptional, IsString } from 'class-validator';
import { Configuration } from '../../../../../shared/types/dtos/scheduler.dto';
import { SchedulerDescriptions } from '../scheduler.constants';

export class SetPointEnforcement {
  @ApiProperty({
    description: SchedulerDescriptions.configuration.setpointEnabled,
  })
  @IsBoolean()
  enabled: boolean;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.setpointFreqSeconds,
  })
  @IsNumber()
  frequency_seconds: number;
}

export class LocalSchedule {
  @ApiProperty({
    description: SchedulerDescriptions.configuration.localScheduleId,
  })
  @IsString()
  id: string;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.localScheduleName,
  })
  @IsString()
  name: string;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.localScheduleClothed,
  })
  @IsBoolean()
  @IsOptional()
  clothed_setpoints?: boolean;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.localScheduleSetPointEnfocement,
  })
  @IsObject()
  setpoint_enforcement: SetPointEnforcement;
}

export class Clients {
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketClientId,
  })
  @IsString()
  id: string;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketClientName,
  })
  @IsString()
  name: string;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketClientIp,
  })
  @IsString()
  ip: string;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketClientPort,
  })
  port: number | string;
}

export class Server {
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketServerEnabled,
  })
  @IsBoolean()
  enabled: boolean;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketServerPort,
  })
  port: number | string;
}

export class WebSockets {
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketsClients,
    type: Clients,
    isArray: true,
  })
  @IsArray()
  clients?: Clients[];
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketServers,
  })
  @IsObject()
  server?: Server;
}

export class Scada {
  @ApiProperty({
    description: SchedulerDescriptions.configuration.scadaStageSize,
  })
  @IsNumber()
  stage_size: number;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.scadaMaxNumConfigs,
  })
  @IsNumber()
  max_num_configurations: number;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.scadaNumFloats,
  })
  @IsNumber()
  num_floats: number;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.scadaNumInts,
  })
  @IsNumber()
  num_ints: number;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.scadaNumBools,
  })
  @IsNumber()
  num_bools: number;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.scadaNumStrings,
  })
  @IsNumber()
  num_strings: number;
}

export class SchedulerConfiguration {
  @ApiProperty({
    description: SchedulerDescriptions.configuration.schedulerType,
  })
  @IsString()
  scheduler_type: string;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.localSchedule,
  })
  @IsOptional()
  local_schedule?: LocalSchedule;
  @ApiProperty({
    description: SchedulerDescriptions.configuration.webSocketsObject,
  })
  @IsOptional()
  web_sockets?: WebSockets;
  @ApiProperty({ description: SchedulerDescriptions.configuration.scada })
  @IsOptional()
  scada?: Scada;
}
