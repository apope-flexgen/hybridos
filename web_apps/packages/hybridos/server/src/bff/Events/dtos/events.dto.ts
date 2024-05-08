import { ApiProperty } from '@nestjs/swagger';
import { IsNumberString, IsOptional } from 'class-validator';
import { EventsRequestParams } from '../../../../../shared/types/dtos/events.dto';
import { EventsDescriptions } from '../events.constants';

export class EventsRequest {
  @ApiProperty({ description: EventsDescriptions.startTime })
  @IsOptional()
  startTime: EventsRequestParams['startTime'];
  @ApiProperty({ description: EventsDescriptions.endTime })
  @IsOptional()
  endTime: EventsRequestParams['endTime'];
  @ApiProperty({ description: EventsDescriptions.source })
  @IsOptional()
  source: EventsRequestParams['source'];
  @ApiProperty({ description: EventsDescriptions.severity })
  @IsOptional()
  severity: EventsRequestParams['severity'];
  @ApiProperty({ description: EventsDescriptions.search })
  @IsOptional()
  search: EventsRequestParams['search'];
  @ApiProperty({ description: EventsDescriptions.limit })
  @IsOptional()
  @IsNumberString()
  limit: EventsRequestParams['limit'];
  @ApiProperty({ description: EventsDescriptions.page })
  @IsOptional()
  @IsNumberString()
  page: EventsRequestParams['page'];
  @ApiProperty({ description: EventsDescriptions.order })
  @IsOptional()
  @IsNumberString()
  order: EventsRequestParams['order'];
  @ApiProperty({ description: EventsDescriptions.orderBy })
  @IsOptional()
  orderBy: EventsRequestParams['orderBy'];
}
