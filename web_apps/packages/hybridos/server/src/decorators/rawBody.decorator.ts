import { BadRequestException, createParamDecorator, ExecutionContext } from '@nestjs/common';
import { ROUTE_ARGS_METADATA } from '@nestjs/common/constants';
import { ApiBody, ApiConsumes, ApiParam } from '@nestjs/swagger';
import rawBody from 'raw-body';

export const RawBody = createParamDecorator(
  async (required: RawBodyRequirement, ctx: ExecutionContext) => {
    const request = ctx.switchToHttp().getRequest<import('express').Request>();

    if (!request.readable) {
      throw new BadRequestException('application/json not accepted, plain/text required');
    }

    const buffer = await rawBody(request);
    if (buffer.length === 0) {
      if (required === RawBodyRequirement.REQUIRED) {
        throw new BadRequestException('Missing required body');
      }
      return undefined;
    }

    const body = buffer.toString('utf-8').trim();
    return body;
  },
  [
    (target: any, key: string) => {
      const metadata = Reflect.getOwnMetadata(ROUTE_ARGS_METADATA, target.constructor, key);
      const [, paramData]: any = Object.values(metadata);
      ApiBody({
        required: paramData.data === RawBodyRequirement.REQUIRED,
        description: 'Body formatted as plain text',
        examples: {
          'naked value': {
            value: true,
            description: 'pass just the raw value',
          },
          'clothed value': {
            value: {
              value: true,
            },
            description: 'pass the value as a JSON object',
          },
        },
      })(target, key, Object.getOwnPropertyDescriptor(target, key));
      ApiConsumes('text/plain')(target, key, Object.getOwnPropertyDescriptor(target, key));
    },
  ],
);

export enum RawBodyRequirement {
  REQUIRED = 'required',
  OPTIONAL = 'optional',
}
