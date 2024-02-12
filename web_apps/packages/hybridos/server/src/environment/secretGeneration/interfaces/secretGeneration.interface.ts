export const SECRET_GENERATION_SERVICE = 'SecretGenerationService';

export interface ISecretGenerationService {
  generateSecret(format: BufferEncoding, length: number): string;
}
