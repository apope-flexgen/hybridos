import { TreeConfig } from 'shared/types/dtos/configurablePages.dto';

export type SiteDiagramConfigBody = {
  _doc?: string;
  _id?: string;
  _version?: string;
  baseURI?: string;
  assets: SiteDiagramData;
  tree: { root: TreeConfig };
};

export interface TemplatedItem {
  treeId: string;
  name: string;
  uri: string;
}

export type SiteDiagramData = { [assetType: string]: AssetTypeFields };

export type AssetTypeFields = {
  baseURI?: string;
  items: AssetInfo[];
};

export type Template = {
  token: string;
  type: 'sequential' | 'list' | 'range';
  list?: string[];
  range?: (string | number)[];
  to?: number;
  from?: number;
  minWidth?: number;
};

export type AssetInfo = {
  name: string;
  uri: string;
  treeId: string;
  templates?: Template[];
  statuses: Status[];
};

export type Status = {
  name: string;
  scalar: string | null;
  units: string;
  uri: string;
  type?: string;
};
