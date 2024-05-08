import { Type } from 'class-transformer';
import { IsNotEmpty, IsOptional, ValidateNested } from 'class-validator';
import { ControllerThrottleConfig } from './ControllterThrottleConfig';
import { ThrottleData } from './ThrottleData';

export class AppThrottleConfig {
  @IsNotEmpty()
  @ValidateNested()
  @Type(() => ThrottleData)
  default_throttle: ThrottleData;

  @IsOptional()
  @Type(() => ControllerThrottleConfig)
  // Currently not used. For future use when nestjs makes throttle
  // values configurable at the controller / method level.
  controllers: Map<string, ControllerThrottleConfig>;

  constructor(ttl: number, limit: number) {
    this.default_throttle = new ThrottleData(ttl, limit);
    this.controllers = new Map<string, ControllerThrottleConfig>();
  }
}
