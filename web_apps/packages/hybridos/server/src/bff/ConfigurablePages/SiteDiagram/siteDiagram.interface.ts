import { Observable } from 'rxjs';
import { ConfigurablePageDTO, DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto';
import { AssetInfo, SiteDiagramConfigBody, Status, TemplatedItem } from './siteDiagram.types';

export const SITE_DIAGRAM_SERVICE = 'SiteDiagramService';

export interface ISiteDiagramService {
  subscribeToSiteDiagram(siteId?: string): Promise<Observable<ConfigurablePageDTO>>;

  getSiteDiagramConfig(): Promise<SiteDiagramConfigBody>;

  getFleetSiteDiagrams(siteId: string): Promise<SiteDiagramConfigBody>;

  getFleetSiteIds(): Promise<string[]>;

  getStaticData(): Promise<ConfigurablePageDTO>;

  parseStaticAssetStatuses(statuses: Status[]): DisplayGroupDTO['status'];

  getTemplatedItems(item: AssetInfo): TemplatedItem[];

  getStaticObservable: (data: ConfigurablePageDTO) => Observable<ConfigurablePageDTO>;

  getMergedStream(data: ConfigurablePageDTO): Observable<ConfigurablePageDTO>;

  subscribeToUri(uri: string, staticData: DisplayGroupDTO): Observable<ConfigurablePageDTO>;
}
