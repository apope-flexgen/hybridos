import { IsString, IsNotEmpty, IsBoolean } from 'class-validator';

export class LockModeDTO {
  @IsNotEmpty()
  @IsString()
  uri: string;

  @IsBoolean()
  value: boolean;
}
