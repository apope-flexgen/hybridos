// TODO: fix lint
/* eslint-disable react/no-unused-prop-types */
import { FC, createElement } from 'react';
import {
  NavigateFunction, Route, Routes, useNavigate,
} from 'react-router-dom';
import NotFound from 'src/pages/NotFound';

export interface RouteProps {
  componentName: string;
  /**  what path to follow when user clicks on this tab */
  path: string;
  /** icon to display on tab */
  icon: string;
  /** what name to display on the tab */
  itemName: string;
  /** key for ui config assets */
  assetKey?: string;
  /** whether a divider should be displayed */
  showDivider?: boolean;
}

export interface RoutesProps {
  routes: Array<RouteProps>;
  pageDictionary: any;
  currentUser: any;
  product: string | null;
}

const componentFactory = (
  component: any,
  currentUser: any,
  pageName: string,
  product: string | null,
  navigator: NavigateFunction,
  assetKey?: string,
) => createElement(component as FC, {
  ...{},
  // TODO: fix lint
  // eslint-disable-next-line @typescript-eslint/ban-ts-comment
  // @ts-ignore
  pageName,
  currentUser,
  product,
  navigator,
  assetKey,
});

const AppRoutes = ({
  routes, pageDictionary, currentUser, product,
}: RoutesProps): JSX.Element => {
  const navigator = useNavigate();

  return (
    <Routes>
      {routes.map(({
        componentName, itemName, path, assetKey,
      }: RouteProps, i) => {
        // FIXME: should this stay here? it is a bandaid, but a more permanent check for this with an error could be good
        if (!componentName || !pageDictionary[componentName]) return null;
        return (
          <Route
            element={componentFactory(
              pageDictionary[componentName],
              currentUser,
              itemName,
              product,
              navigator,
              assetKey,
            )}
            // TODO: fix lint
            // eslint-disable-next-line react/no-array-index-key
            key={i}
            path={path}
          />
        );
      })}
      {/* 👇️ only match this when no other routes match */}
      {routes.length && <Route path="*" element={<NotFound />} />}
    </Routes>
  );
};

export default AppRoutes;
