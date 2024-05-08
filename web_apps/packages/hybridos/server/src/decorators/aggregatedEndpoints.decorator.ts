import { SetMetadata } from '@nestjs/common';

export const IS_AGGREGATED_ENDPOINTS_KEY = 'isAggregatedEndpoints';
export const AggregatedEndpoints = () => SetMetadata(IS_AGGREGATED_ENDPOINTS_KEY, true);
