import { Injectable } from '@nestjs/common'
import { InjectModel } from '@nestjs/sequelize'
import { Transaction } from 'sequelize'
import { UserDto } from './dto/user.dto'
import { User } from './user.model'

@Injectable()
export class UsersService {
    constructor(
        @InjectModel(User)
        private userModel: typeof User
    ) {}
    async findAll(): Promise<User[]> {
        return this.userModel.findAll()
    }
    findOne(id: string): Promise<User> {
        return this.userModel.findOne({
            where: {
                id,
            },
        })
    }
    async remove(id: string): Promise<void> {
        const user = await this.findOne(id)
        await user.destroy()
    }
    async create(userDto: UserDto, transaction?: Transaction): Promise<User> {
        return await this.userModel.create(
            {
                username: userDto.username,
                firstName: userDto.firstName,
                lastName: userDto.lastName,
                isActive: userDto.isActive,
            },
            { transaction: transaction }
        )
    }
    async findOrCreate(options: any, transaction: Transaction): Promise<User> {
        const existing = await this.userModel.findOne({
            where: { username: options.username },
            transaction: transaction,
        })
        if (existing) return existing

        const created = await this.create(options, transaction)
        return created
    }
}
