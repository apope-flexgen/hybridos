import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsOptional, IsString } from 'class-validator';
import { AlertsDescriptions } from '../alerts.constants';

export class Organization {
  @IsOptional()
  id?: string;
  @IsString()
  name: string;
}
export class OrganizationsDTO {
  @ApiProperty({ description: AlertsDescriptions.organizations })
  @IsArray()
  organizations: Organization[];
}
