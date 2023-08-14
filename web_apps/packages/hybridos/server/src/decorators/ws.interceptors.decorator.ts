import { applyDecorators, NestInterceptor, UseInterceptors } from '@nestjs/common';
import { SocketNamespaceInterceptor } from '../interceptors/socketNamespace.interceptor';

export const UseWsInterceptors = (...interceptors: (Function | NestInterceptor<any, any>)[]) => {
  return applyDecorators(UseInterceptors(SocketNamespaceInterceptor, ...interceptors));
};
