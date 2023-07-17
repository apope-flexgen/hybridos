/* eslint-disable react/no-unstable-nested-components, no-nested-ternary, max-lines, max-len */
// TODO: fix lint
import { PageLoadingIndicator, lightTheme } from '@flexgen/storybook';
import {
  Dispatch,
  SetStateAction,
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
} from 'react';
import { Layout } from 'shared/types/dtos/layouts.dto';
import { SiteConfiguration } from 'shared/types/dtos/siteConfig.dto';
import { AUTH_USER_TOKEN_URL } from 'src/App/helpers/constants';
import BaseApp from 'src/components/BaseApp';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { Login } from 'src/pages';
import QueryService from 'src/services/QueryService';
import { ThemeProvider } from 'styled-components';

interface AppContextValue {
  currentUser: any;
  setCurrentUser: Dispatch<SetStateAction<never[]>>;
  product: string | null;
  setProduct: Dispatch<SetStateAction<string | null>>;
  siteConfiguration: SiteConfiguration | null;
  setSiteConfiguration: Dispatch<SetStateAction<SiteConfiguration | null>>;
  layouts: Layout[];
  setLayouts: Dispatch<SetStateAction<Layout[]>>;
  loggedIn: boolean;
  setLoggedIn: Dispatch<SetStateAction<boolean>>;
}

const AppContext = createContext<AppContextValue>({
  currentUser: undefined,
  setCurrentUser: () => {},
  product: null,
  setProduct: () => {},
  siteConfiguration: null,
  setSiteConfiguration: () => {},
  layouts: [],
  setLayouts: () => {},
  loggedIn: false,
  setLoggedIn: () => {},
});

export function useAppContext() {
  return useContext(AppContext);
}

const App = (): JSX.Element => {
  const [currentUser, setCurrentUser] = useState<any>(undefined);
  const [loggedIn, setLoggedIn] = useState<boolean>(false);
  const [siteConfiguration, setSiteConfiguration] = useState<SiteConfiguration | null>(null);
  const [product, setProduct] = useState<string | null>(null);
  const [layouts, setLayouts] = useState<Layout[]>([]);
  const [isLoading, setIsLoading] = useState(true);
  const axiosInstance = useAxiosWebUIInstance(true);

  const fetchUserData = useCallback(async () => {
    try {
      const res = await axiosInstance.get(AUTH_USER_TOKEN_URL);
      const user = res.data;
      setCurrentUser(user);
      setLoggedIn(true);
    } finally {
      QueryService.cleanupSocket();
      setIsLoading(false);
    }
  }, [axiosInstance]);

  const contextValue = useMemo(
    () => ({
      currentUser,
      setCurrentUser,
      siteConfiguration,
      setSiteConfiguration,
      product,
      setProduct,
      layouts,
      setLayouts,
      loggedIn,
      setLoggedIn,
    }),
    [
      currentUser,
      setCurrentUser,
      siteConfiguration,
      setSiteConfiguration,
      product,
      setProduct,
      layouts,
      setLayouts,
      loggedIn,
      setLoggedIn,
    ],
  );

  useEffect(() => {
    fetchUserData();
  }, [fetchUserData]);

  return (
    <AppContext.Provider value={contextValue}>
      {isLoading ? (
        <ThemeProvider theme={lightTheme}>
          <PageLoadingIndicator isLoading type="primary" />
        </ThemeProvider>
      ) : loggedIn ? (
        <BaseApp />
      ) : (
        <Login />
      )}
    </AppContext.Provider>
  );
};

export default App;
