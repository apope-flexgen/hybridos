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
}

export type LoginInfo = {
  product: string;
}

