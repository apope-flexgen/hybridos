export type SiteConfiguration = {
    timezone: string; 
    product: string;
    ess: boolean;
    gen: boolean;
    solar: boolean;
    met_station: boolean;
    tracker: boolean;
    feeders: boolean;
    features: boolean;
    site_status_bar?: boolean;
    system_status?: boolean;
    site: boolean;
    events: boolean;
    control_cabinet: boolean;
    fleet_manager_dashboard: boolean;
    scheduler: boolean;
    units: {
      [key: string]: string;
    },
    inspectorComponentsName: string;
    site_name?: string;
    fleet_name?: string;
    customer?: {
      name?: string,
      server?: string
    }
}

export type LoginInfo = {
  product: string;
  siteName?: string;
  customer?: string;
  server?: string;
}

