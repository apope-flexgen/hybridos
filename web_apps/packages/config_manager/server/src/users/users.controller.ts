import { Body, Controller, Get, Param, Post } from '@nestjs/common'
import { UserDto } from './dto/user.dto'
import { ReadUserParams } from './params/readuser.params'
import { UsersService } from './users.service'

@Controller('users')
export class UsersController {
    constructor(private readonly usersService: UsersService) {}
    @Get()
    async getAll() {
        return await this.usersService.findAll()
    }
    @Get(':id')
    async read(@Param() params: ReadUserParams) {
        return await this.usersService.findOne(params.id)
    }
    @Post()
    async create(@Body() userDto: UserDto) {
        return await this.usersService.create(userDto)
    }
}
