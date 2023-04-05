import { Controller, Get } from '@nestjs/common'

@Controller('permissions')
export class PermissionsController {
    @Get()
    findAll(): string {
        return 'This action returns all permissions'
    }
}
