export interface LockModeState {
  currentState: boolean;
  hasChanged: boolean;
  username?: string;
}

export type DBILockModeState = {
  [category: string]: {
    [assetID: string]: IndividualDBILockModeState;
  };
};

export type IndividualDBILockModeState = {
  value: boolean;
  username?: string;
};
