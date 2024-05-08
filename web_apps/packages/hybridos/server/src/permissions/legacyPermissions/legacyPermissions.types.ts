export interface LegacyPermissions {
  authentication: {
    notes: string;
    permissions: LegacyRole[];
  };
}

export interface LegacyRole {
  roleOrUsername: string;
  notes: string;
  access: string[];
}
