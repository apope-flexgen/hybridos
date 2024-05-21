import { Inject, Injectable } from '@nestjs/common';
import { FimsService } from '../../../fims/fims.service';
import { FIMS_SERVICE } from '../../../fims/interfaces/fims.interface';
import { DBI_URIs, IDBIService } from 'src/dbi/dbi.interface';
import { DBI_SERVICE } from 'src/dbi/dbi.interface';
import { Observable, map, merge } from 'rxjs';
import { DBIDocumentNotFoundException } from 'src/dbi/exceptions/dbi.exceptions';
import {
  AssetInfo,
  SiteDiagramConfigBody,
  Status,
  Template,
  TemplatedItem,
} from './siteDiagram.types';
import { ConfigurablePageDTO, DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto';
import { summaryInfoFromStream } from '../Dashboard/dashboard.types';
import { computeNakedValue } from 'src/utils/utils';
import { generateTemplatedExtensions } from '../Templating/templates.helper';
import { ISiteDiagramService } from './siteDiagram.interface';
import { ConfigNotFoundException } from 'src/exceptions/configNotFound.exception';

@Injectable()
export class SiteDiagramService implements ISiteDiagramService {
  scalarStore: { [fullURI: string]: number } = {};
  unitStore: { [fullURI: string]: string } = {};
  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: FimsService,
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
  ) {}
  subscribeToSiteDiagram = async (siteId?: string): Promise<Observable<ConfigurablePageDTO>> => {
    const staticData = await this.getStaticData(siteId);

    return this.getMergedStream(staticData);
  };

  async getFleetSiteDiagrams(siteId: string): Promise<SiteDiagramConfigBody> {
    const fullFleetConfig = await this.dbiService.getFromDBI(
      DBI_URIs.UI_Config_Fleet_Site_Diagrams,
    );
    return fullFleetConfig[siteId.replace(/\b\w/g, (char: string) => char.toLowerCase())];
  }

  async getFleetSiteIds() {
    const fullFleetConfig = await this.dbiService.getFromDBI(
      DBI_URIs.UI_Config_Fleet_Site_Diagrams,
    );
    return Object.keys(fullFleetConfig);
  }

  getSiteDiagramConfig = async (): Promise<SiteDiagramConfigBody> => {
    try {
      const configData = await this.dbiService.getFromDBI(DBI_URIs.UI_Config_Site_Diagram);
      return configData as unknown as SiteDiagramConfigBody;
    } catch (e) {
      if (e instanceof DBIDocumentNotFoundException) {
        throw new ConfigNotFoundException(e.message);
      }
      throw e;
    }
  };

  private addToUnitScalarStore = (statuses: Status[], fullURI: string) => {
    statuses.forEach((status) => {
      const { uri, scalar, units } = status;
      this.scalarStore[fullURI + uri] = !isNaN(Number(scalar)) ? Number(scalar) : 1;
      this.unitStore[fullURI + uri] = units ?? '';
    });
  };

  getStaticData = async (siteId?: string): Promise<ConfigurablePageDTO> => {
    const configData = siteId
      ? await this.getFleetSiteDiagrams(siteId)
      : await this.getSiteDiagramConfig();
    const topLevelBaseURI = configData.baseURI || '';

    let displayGroups: ConfigurablePageDTO['displayGroups'] = {};

    Object.values(configData.assets).forEach((assetTypeValues) => {
      const assetLevelBaseURI = assetTypeValues.baseURI || '';

      if (!assetTypeValues.items) return;

      assetTypeValues.items.forEach((item) => {
        // handle templated items
        if (item.templates && item.templates.length !== 0) {
          let templatedItems: TemplatedItem[] = this.getTemplatedItems(item);

          if (templatedItems.length === 0) {
            console.warn(
              `Templates were provided for site diagram node with name ${item.name}, but tokens listed were never used.`,
            );

            // if templates were provided, but none of the tokens specified were used to template anything,
            // treat this as a non-templated asset
            templatedItems = [item];
          }

          templatedItems.forEach((templatedItem) => {
            if (!item.statuses) return;

            this.addToUnitScalarStore(
              item.statuses,
              `${topLevelBaseURI}${assetLevelBaseURI}${templatedItem.uri}`,
            );

            const displayGroupEntry = {
              displayName: templatedItem.name,
              treeId: templatedItem.treeId,
              status: this.parseStaticAssetStatuses(item.statuses),
            };

            displayGroups = {
              ...displayGroups,
              [topLevelBaseURI + assetLevelBaseURI + templatedItem.uri]: displayGroupEntry,
            };
          });
        }
        // handle non-templated items
        else {
          this.addToUnitScalarStore(
            item.statuses,
            `${topLevelBaseURI}${assetLevelBaseURI}${item.uri}`,
          );

          const displayGroupEntry = {
            displayName: item.name,
            treeId: item.treeId,
            status: this.parseStaticAssetStatuses(item.statuses),
          };

          displayGroups = {
            ...displayGroups,
            [topLevelBaseURI + assetLevelBaseURI + item.uri]: displayGroupEntry,
          };
        }
      });
    });

    // only return the tree from the config file once, when hasStatic is true
    return {
      hasStatic: true,
      tree: configData.tree.root,
      displayGroups,
    };
  };

  parseStaticAssetStatuses = (statuses: Status[]): DisplayGroupDTO['status'] => {
    const statusDTOs: DisplayGroupDTO['status'] = {};

    statuses?.forEach((status) => {
      const { name, units, uri, type } = status;
      const uriWithOpeningSlashRemoved = uri.replace(/^\//, '');

      statusDTOs[uriWithOpeningSlashRemoved] = {
        static: {
          label: name,
          unit: units,
          type: type,
        },
        state: {
          value: '-',
        },
      };
    });

    return statusDTOs;
  };

  private constructExtensions = (field: string, templates: Template[]) => {
    const template = templates.find((template) => field.includes(template.token));
    return template ? { [field]: generateTemplatedExtensions(field, template) } : {};
  };

  getTemplatedItems = (item: AssetInfo): TemplatedItem[] => {
    const { templates, treeId, name, uri } = item;

    const templatedExtensionArrays = {
      ...this.constructExtensions(uri, templates),
      ...this.constructExtensions(treeId, templates),
      ...this.constructExtensions(name, templates),
    };

    const shortestTemplate = Object.values(templatedExtensionArrays).reduce((prev, next) => {
      if (prev.length && prev.length != next.length) {
        console.warn(
          `A template entered for site diagram node with name ${item.name} are not all of the same length. The minimum length will be used.`,
        );
      }
      return prev.length > next.length ? next : prev;
    });

    return Array.from({ length: shortestTemplate.length }, (_, index) => ({
      treeId: templatedExtensionArrays[treeId]?.[index] ?? treeId,
      name: templatedExtensionArrays[name]?.[index] ?? name,
      uri: templatedExtensionArrays[uri]?.[index] ?? uri,
    }));
  };

  getStaticObservable = (data: ConfigurablePageDTO): Observable<ConfigurablePageDTO> => {
    return new Observable((subscriber) => {
      subscriber.next(data);
    });
  };

  getMergedStream = (data: ConfigurablePageDTO): Observable<ConfigurablePageDTO> => {
    const uris = Object.keys(data.displayGroups);

    const staticObservable: Observable<ConfigurablePageDTO> = this.getStaticObservable(data);

    const dataStreamObservables = uris.map((uri) =>
      this.subscribeToUri(uri, data.displayGroups[uri]),
    );

    const merged: Observable<ConfigurablePageDTO> = merge(
      staticObservable,
      ...dataStreamObservables,
    );

    return merged;
  };

  computeRealValue = (value: number | string | boolean, uri: string, key: string) => {
    if (typeof value !== 'number') {
      return { value };
    }
    const scalar = this.scalarStore[uri + '/' + key] || 1;
    const units = this.unitStore[uri + '/' + key] || '';
    return computeNakedValue(value, scalar, units);
  };

  subscribeToUri = (uri: string, staticData: DisplayGroupDTO): Observable<ConfigurablePageDTO> => {
    const fimsSubscribe = this.fimsService.subscribe(uri);

    return fimsSubscribe.pipe(
      map((event) => {
        // handle new data coming in from subscribe
        const transformed: ConfigurablePageDTO = {
          hasStatic: false,
          displayGroups: {
            [uri]: {
              displayName: staticData.displayName,
              treeId: staticData.treeId,
              status: {},
            },
          },
        };

        const rawData = event.body as summaryInfoFromStream;

        Object.entries(staticData.status).forEach(([key, _]) => {
          const componentID = key;

          if (rawData[componentID] === undefined || rawData[componentID] === null) {
            return;
          }

          const rawValue = rawData[componentID];

          const value: number | string | boolean =
            rawValue !== null && typeof rawValue === 'object'
              ? rawValue.value
              : (rawValue as number | string | boolean);

          const { value: realValue } = this.computeRealValue(value, uri, key);

          transformed.displayGroups[uri].status[key] = {
            state: {
              value: realValue,
            },
          };
        });

        return transformed;
      }),
    );
  };
}
