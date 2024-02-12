import { CallHandler, ExecutionContext, Injectable, NestInterceptor } from '@nestjs/common';
import { map, Observable } from 'rxjs';
import { ConfigurablePageDTO, DashboardLayout } from 'shared/types/dtos/configurablePages.dto';
import {
  TableDashboardDataTableDTO,
  TableDashboardDataTableIndexable,
} from 'shared/types/dtos/dataTables.dto';
@Injectable()
export class DashboardLayoutInterceptor implements NestInterceptor {
  constructor() {}
  intercept(
    context: ExecutionContext,
    next: CallHandler<any>,
  ): Observable<TableDashboardDataTableDTO> | Promise<Observable<TableDashboardDataTableDTO>> {
    const data = context.switchToWs().getData();
    const layout: DashboardLayout = data.data;

    switch (layout) {
      case DashboardLayout.TABLE:
        return next.handle().pipe(map(this.parseForTableLayout), map(this.convertToDataTableDTO));
      case DashboardLayout.CARD:
      default:
        return next.handle();
    }
  }

  parseForTableLayout = (data: ConfigurablePageDTO): TableDashboardDataTableIndexable => {
    const toReturn: TableDashboardDataTableIndexable = {};
    Object.entries(data.displayGroups).forEach(([_, groupData]) => {
      const divIndex = groupData.displayName.indexOf('-');

      const tableName =
        divIndex < 0 ? groupData.displayName : groupData.displayName.slice(0, divIndex).trim();
      const rowName =
        divIndex < 0 ? groupData.displayName : groupData.displayName.slice(divIndex + 1).trim();

      if (!(tableName in toReturn)) {
        toReturn[tableName] = {
          displayName: tableName,
          columns: data.hasStatic ? { id: { id: 'id', label: '' } } : null,
          rows: {},
          batteryViewData: {},
        };
      }

      if (groupData.batteryViewStatus !== undefined) {
        Object.entries(groupData.batteryViewStatus).forEach(([statusID, statusInfo]) => {
          if ('state' in statusInfo) {
            toReturn[tableName].batteryViewData = {
              ...toReturn[tableName].batteryViewData,
              [rowName]: { label: rowName, value: statusInfo.state.value },
            };
          }
        });
      }

      Object.entries(groupData.status).forEach(([statusID, statusInfo]) => {
        if ('static' in statusInfo) {
          if (!(statusID in toReturn[tableName].columns)) {
            toReturn[tableName].columns[statusID] = {
              id: statusID,
              label: statusInfo.static.unit
                ? `${statusInfo.static.label} (${statusInfo.static.unit})`
                : statusInfo.static.label,
              minWidth: 125,
            };
          }
        }
        if ('state' in statusInfo) {
          if (!(rowName in toReturn[tableName].rows)) {
            toReturn[tableName].rows[rowName] = { id: rowName };
          }
          toReturn[tableName].rows[rowName][statusID] = statusInfo.state.value;
        }
      });
    });
    return toReturn;
  };

  convertToDataTableDTO = (data: TableDashboardDataTableIndexable): TableDashboardDataTableDTO => {
    const toReturn: TableDashboardDataTableDTO = {};
    Object.entries(data).forEach(([tableName, tableData]) => {
      toReturn[tableName] = {
        displayName: tableData.displayName,
        columns: tableData.columns !== null ? Object.values(tableData.columns) : undefined,
        rows: Object.values(tableData.rows),
        batteryViewData: Object.values(tableData.batteryViewData),
      };
    });
    return toReturn;
  };
}
