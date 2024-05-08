import { ApiProperty } from '@nestjs/swagger';
import { AssetsDescriptions } from './assets.constants';

export class AssetsResponse {
  @ApiProperty({ description: AssetsDescriptions.assetsResponse })
  assets: string | Record<string, unknown>;
}
