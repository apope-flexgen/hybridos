/* eslint-disable max-nested-callbacks */
import { Test, TestingModule } from '@nestjs/testing';
import {
  ISecretGenerationService,
  SECRET_GENERATION_SERVICE,
} from 'src/environment/secretGeneration/interfaces/secretGeneration.interface';
import { SecretGenerationService } from 'src/environment/secretGeneration/secretGeneration.service';

describe('DefaultUserService', () => {
  let secretGenerationService: ISecretGenerationService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [{ provide: SECRET_GENERATION_SERVICE, useClass: SecretGenerationService }],
    }).compile();

    secretGenerationService = module.get<ISecretGenerationService>(SECRET_GENERATION_SERVICE);
  });

  describe('generate secret', () => {
    it('should generate a string of specified length', () => {
      const length = 32;
      const result = secretGenerationService.generateSecret('base64', length);

      expect(result).toHaveLength(32);
    });
  });
});
