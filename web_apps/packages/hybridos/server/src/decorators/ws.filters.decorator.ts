import { applyDecorators, ExceptionFilter, UseFilters } from '@nestjs/common';
import { CoreWsExceptionsFilter } from '../filters/core.ws.exception.filter';

export const UseWsFilters = (...filters: (Function | ExceptionFilter<any>)[]) => {
  return applyDecorators(UseFilters(CoreWsExceptionsFilter, ...filters));
};
