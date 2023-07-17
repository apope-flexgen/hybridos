import { Inject, Injectable } from '@nestjs/common';
import { DBI_SERVICE, DBI_URIs, IDBIService } from '../../../../dbi/dbi.interface';
import { DBIDocumentNotFoundException } from '../../../../dbi/exceptions/dbi.exceptions';
import { DBILockModeState, IndividualDBILockModeState, LockModeState } from './lockMode.types';

@Injectable()
export class LockModeService {
  lockModeStateMap: {
    [key: string]: LockModeState;
  };

  constructor(
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
  ) {
    this.lockModeStateMap = {};
  }

  // called by the parser to track the current state from fims
  setStateForURI = (URI: string, state: boolean) => {
    const hasChanged: boolean =
      !(URI in this.lockModeStateMap) || this.lockModeStateMap[URI].currentState !== state;

    this.lockModeStateMap[URI] = {
      ...this.lockModeStateMap[URI],
      currentState: state,
      ...(hasChanged && { hasChanged }),
    };
  };

  // called by the interceptor to get the current state and access
  getLockModeObject = async (URI: string, username: string, enabledAssetPageControls) => {
    if (URI in this.lockModeStateMap && this.lockModeStateMap[URI].hasChanged) {
      this.lockModeStateMap[URI].username = await this.queryForUsername(URI);
      this.lockModeStateMap[URI].hasChanged = false;
    }

    const lockingUsername: string | undefined = this.lockModeStateMap[URI]?.username;
    const userHasAccess: boolean | undefined =
      lockingUsername === undefined ? undefined : lockingUsername === username;

    return {
      username: lockingUsername,
      lockModeButtonEnabled: enabledAssetPageControls ? userHasAccess : false,
      isLocked: this.lockModeStateMap[URI]?.currentState,
    };
  };

  private queryForUsername = async (URI: string): Promise<string | undefined> => {
    const lockModeStatus = await this.getLockModeStatus(URI);

    return lockModeStatus?.username;
  };

  // called by gateway to check if a user has access to lock/unlock an asset
  canChangeLockMode = async (
    URI: string,
    username: string,
    enabledAssetPageControls: boolean,
  ): Promise<boolean> => {
    if (!enabledAssetPageControls) return false;

    const lockModeStatus = await this.getLockModeStatus(URI);

    return lockModeStatus?.username === undefined || lockModeStatus.username === username;
  };

  private getLockModeStatus = async (
    URI: string,
  ): Promise<IndividualDBILockModeState | undefined> => {
    const { category, assetID } = this.getCategoryAndAssetID(URI);

    try {
      const dbiLockModeState: DBILockModeState = await this.dbiService.getFromDBI(
        DBI_URIs.LOCK_MODE_STATE,
      );

      return dbiLockModeState?.[category]?.[assetID];
    } catch (e) {
      if (e instanceof DBIDocumentNotFoundException) return undefined;

      throw e;
    }
  };

  postLockModeUpdate = async (URI: string, state: boolean, username: string) => {
    const { category, assetID } = this.getCategoryAndAssetID(URI);

    const lockModeState = await this.getCurrentLockModeState();

    lockModeState[category] = lockModeState[category] ?? {};

    lockModeState[category][assetID] = {
      value: state,
      ...(state ? { username: username } : {}),
    };

    await this.dbiService.postToDBI(DBI_URIs.LOCK_MODE_STATE, lockModeState);
  };

  private getCategoryAndAssetID = (URI: string): { category: string; assetID: string } => {
    const splitURI = URI.split('/');
    const category = splitURI[2];
    const assetID = splitURI[3];

    return { category, assetID };
  };

  private getCurrentLockModeState = async (): Promise<DBILockModeState> => {
    try {
      const res = await this.dbiService.getFromDBI(DBI_URIs.LOCK_MODE_STATE);
      return res;
    } catch (e) {
      if (e instanceof DBIDocumentNotFoundException) return {};

      throw e;
    }
  };
}
