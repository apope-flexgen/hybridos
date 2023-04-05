import { Module } from '@nestjs/common'
import { SequelizeModule } from '@nestjs/sequelize'
import { ConfigFilesModule } from './configFiles/configFiles.module'
import { MongooseModule } from '@nestjs/mongoose'
import { UsersModule } from './users/users.module'
import { AppInitModule } from './appInit/appInit.module'

@Module({
    imports: [
        SequelizeModule.forRoot({
            dialect: 'mariadb',
            host: 'localhost',
            port: 3306,
            username: 'root',
            password: 'root',
            database: 'config_manager',
            sync: { force: true },
            synchronize: true,
            autoLoadModels: true,
        }),
        MongooseModule.forRoot('mongodb://localhost:27017/configMgr'),
        UsersModule,
        ConfigFilesModule,
        AppInitModule,
    ],
    controllers: [],
    providers: [],
})
export class AppModule {}
