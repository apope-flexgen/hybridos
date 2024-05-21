import { Module } from '@nestjs/common';
import { FimsModule } from '../../../fims/fims.module';
import { DBIModule } from 'src/dbi/dbi.module';
import { SiteDiagramService } from './siteDiagram.service';
import { SiteDiagramGateway } from './siteDiagram.gateway';
import { SITE_DIAGRAM_SERVICE } from './siteDiagram.interface';
import { SiteDiagramController } from './siteDiagram.controller';
@Module({
  imports: [FimsModule, DBIModule],
  controllers: [SiteDiagramController],
  providers: [{ provide: SITE_DIAGRAM_SERVICE, useClass: SiteDiagramService }, SiteDiagramGateway],
})
export class SiteDiagramModule {}
