import { Global, Module } from '@nestjs/common';
import { SECRET_GENERATION_SERVICE } from 'src/environment/secretGeneration/interfaces/secretGeneration.interface';
import { SecretGenerationService } from 'src/environment/secretGeneration/secretGeneration.service';

@Global()
@Module({
  imports: [],
  providers: [{ provide: SECRET_GENERATION_SERVICE, useClass: SecretGenerationService }],
  exports: [SECRET_GENERATION_SERVICE],
})
export class SecretGenerationModule {}
