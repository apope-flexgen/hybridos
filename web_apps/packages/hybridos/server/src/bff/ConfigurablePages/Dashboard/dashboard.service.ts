import { Inject, Injectable } from '@nestjs/common';
import { map, merge, Observable } from 'rxjs';
import { FimsService } from '../../../fims/fims.service';
import { FIMS_SERVICE } from '../../../fims/interfaces/fims.interface';
import { computeNakedValue } from '../../../utils/utils';
import { ConfigurablePageDTO, DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto';
import {
  summaryInfoFromStream,
  ConfigBody,
  SingleCardData,
  SingleStatus,
  SingleItemInfo,
} from './dashboard.types';
import { DBI_URIs, IDBIService } from 'src/dbi/dbi.interface';
import { DBI_SERVICE } from 'src/dbi/dbi.interface';
import { DashboardConfigNotFoundException } from './exceptions/dashboard.exceptions';
import { DBIDocumentNotFoundException } from '../../../dbi/exceptions/dbi.exceptions';

@Injectable()
export class DashboardService {
  configData: SingleCardData[] = [];
  scalarStore: { [fullURI: string]: number } = {};
  unitStore: { [fullURI: string]: string } = {};
  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: FimsService,
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
  ) {}

  subscribeToDashboard = async (): Promise<Observable<ConfigurablePageDTO>> => {
    const staticData = await this.getStaticData();

    return this.getMergedStream(staticData);
  };

  private getConfigData = async (): Promise<SingleCardData[]> => {
    try {
      const configData = await this.dbiService.getFromDBI(DBI_URIs.UI_Config_Dashboard);

      // FIXME: validate this instead
      return (configData as unknown as ConfigBody).data;
    } catch (e) {
      if (e instanceof DBIDocumentNotFoundException) {
        throw new DashboardConfigNotFoundException(e.message);
      }
      throw e;
    }
  };

  private getStaticData = async (): Promise<ConfigurablePageDTO> => {
    const configData = await this.getConfigData();

    const displayGroups = configData.reduce((acc, card) => {
      // store all scalars for use later. since this is coming from a static
      // config, scalars will not be different between users, so it is safe
      // to store them here.
      const { info, status } = card;
      const baseURIs: SingleItemInfo[] = info.isTemplate
        ? info.items
        : [{ name: info.name, uri: info.baseURI }];

      const sourceURI = info.sourceURIs[0];
      if (info.sourceURIs.length > 1) {
        console.warn(
          'More than one sourceURI provided, only the first will be used. Is this intentional?',
        );
      }

      baseURIs.forEach((baseURI) => {
        status.forEach((status) => {
          const { uri, scalar, units } = status;
          this.scalarStore[sourceURI + baseURI.uri + uri] = Number(scalar) ?? 1;
          this.unitStore[sourceURI + baseURI.uri + uri] = units ?? '';
        });

        const batteryViewStatus = {
          name: `Battery View - ${baseURI.name}`,
          scalar: null,
          sourceURI: info.batteryViewSourceURI,
          units: '',
          uri: info.batteryViewURI,
        };

        acc[sourceURI + baseURI.uri] = {
          displayName: info.name === baseURI.name ? info.name : `${info.name} - ${baseURI.name}`,
          batteryViewStatus:
            info.isTemplate && info.batteryView
              ? this.getStatusDTOs([batteryViewStatus])
              : undefined,
          status: this.getStatusDTOs(status),
        };
      });

      return acc;
    }, {} as ConfigurablePageDTO['displayGroups']);

    return {
      hasStatic: true,
      assetKey: 'dashboard',
      displayGroups,
    };
  };

  private getStatusDTOs = (statuses: SingleStatus[]): DisplayGroupDTO['status'] => {
    return statuses.reduce((acc, status) => {
      const { name, units, uri } = status;

      const uriWithOpeningSlashRemoved = uri.replace(/^\//, '');

      acc[uriWithOpeningSlashRemoved] = {
        static: {
          label: name,
          unit: units,
          variant: 'dynamic',
        },
        state: {
          value: '-',
        },
      };

      return acc;
    }, {} as DisplayGroupDTO['status']);
  };

  private getMergedStream = (data: ConfigurablePageDTO): Observable<ConfigurablePageDTO> => {
    const uris = Object.keys(data.displayGroups);

    const base: Observable<ConfigurablePageDTO> = new Observable((subscriber) => {
      subscriber.next(data);
    });

    const observables = uris.map((uri) =>
      this.subscribeToUri(
        uri,
        data.displayGroups[uri].displayName,
        data.displayGroups[uri].batteryViewStatus || undefined,
      ),
    );

    const merged: Observable<ConfigurablePageDTO> = merge(base, ...observables);

    return merged;
  };

  private subscribeToUri = (
    uri: string,
    displayName: string,
    batteryViewStatuses: DisplayGroupDTO['batteryViewStatus'],
  ): Observable<ConfigurablePageDTO> => {
    const fimsSubscribe = this.fimsService.subscribe(uri);

    return fimsSubscribe.pipe(
      map((event) => {
        const transformed: ConfigurablePageDTO = {
          hasStatic: false,
          assetKey: 'dashboard',
          displayGroups: {
            [uri]: {
              displayName,
              status: {},
              batteryViewStatus: {},
            },
          },
        };

        const rawData = event.body as summaryInfoFromStream;

        Object.entries(rawData).forEach(([key, rawValue]) => {
          const value: number | string | boolean =
            rawValue !== null && typeof rawValue === 'object'
              ? rawValue.value
              : (rawValue as number | string | boolean);

          const { value: trueValue } =
            typeof value === 'number'
              ? (() => {
                  const scalar = this.scalarStore[uri + '/' + key] || 1;
                  const units = this.unitStore[uri + '/' + key] || '';
                  return computeNakedValue(value, scalar, units);
                })()
              : { value };

          if (batteryViewStatuses !== undefined && key in batteryViewStatuses) {
            transformed.displayGroups[uri].batteryViewStatus[key] = {
              state: {
                value: trueValue,
              },
            };
          }
          transformed.displayGroups[uri].status[key] = {
            state: {
              value: trueValue,
            },
          };
        });
        return transformed;
      }),
    );
  };
}
