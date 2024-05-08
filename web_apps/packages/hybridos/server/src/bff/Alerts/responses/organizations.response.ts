import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsOptional, IsString } from 'class-validator';
import { AlertsDescriptions } from '../alerts.constants';

export class Organization {
  @IsOptional()
  id: string;
  @IsString()
  name: string;
}
export class OrganizationsResponse {
  @ApiProperty({ description: AlertsDescriptions.organizations })
  @IsArray()
  data: Organization[];
}
