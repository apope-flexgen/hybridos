import { ApiProperty } from '@nestjs/swagger';
import { IsInt, IsIP, IsPort, IsString, Max, Min } from 'class-validator';
import { SiteAdminsDescriptions } from '../siteAdmins.constants';

export class RadiusTestDto {
  @ApiProperty({ description: SiteAdminsDescriptions.radiusIp })
  @IsIP()
  ip_address: string;
  @ApiProperty({ description: SiteAdminsDescriptions.radiusPort })
  @IsPort()
  port: string;
  @ApiProperty({ description: SiteAdminsDescriptions.radiusSecretPhrase })
  @IsString()
  secret_phrase: string;
  @ApiProperty({ description: SiteAdminsDescriptions.radiusWaitTime })
  @IsInt()
  @Min(0)
  @Max(30000)
  wait_time: number;
  @ApiProperty({ description: SiteAdminsDescriptions.radiusUsername })
  @IsString()
  username: string;
  @ApiProperty({ description: SiteAdminsDescriptions.radiusPassword })
  @IsString()
  password: string;
}
