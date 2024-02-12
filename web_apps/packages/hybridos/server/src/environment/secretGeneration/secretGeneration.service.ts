import { Injectable } from '@nestjs/common';
import { randomBytes } from 'crypto';
import { ISecretGenerationService } from 'src/environment/secretGeneration/interfaces/secretGeneration.interface';

@Injectable()
export class SecretGenerationService implements ISecretGenerationService {
  constructor() {}

  generateSecret = (format: BufferEncoding, length: number): string => {
    return randomBytes(Math.ceil(length)).toString(format).slice(0, length);
  };
}
