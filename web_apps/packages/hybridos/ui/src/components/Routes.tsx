// TODO: fix lint
/* eslint-disable react/no-unused-prop-types */
import { FC, createElement } from 'react';
import {
  NavigateFunction, Route, Routes, useNavigate,
} from 'react-router-dom';
import NotFound from 'src/pages/NotFound';

export interface RouteProps {
  componentName: string
  /**  what path to follow when user clicks on this tab */
  path: string
  /** icon to display on tab */
  icon: string
  /** what name to display on the tab */
  itemName: string
}

export interface RoutesProps {
  routes: Array<RouteProps>
  pageDictionary: any
  currentUser: any
  product: string | null
}

const componentFactory = (
  component: any,
  currentUser: any,
  pageName: string,
  product: string | null,
  navigator: NavigateFunction,
) => createElement(component as FC, {
  ...{},
  // TODO: fix lint
  // eslint-disable-next-line @typescript-eslint/ban-ts-comment
  // @ts-ignore
  pageName,
  currentUser,
  product,
  navigator,
});

const AppRoutes = ({ routes, pageDictionary, currentUser, product }: RoutesProps): JSX.Element => {
  const navigator = useNavigate();

  return (
    <Routes>
      {routes.map(({
        componentName, itemName, path,
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
            )}
            // TODO: fix lint
            // eslint-disable-next-line react/no-array-index-key
            key={i}
            path={path}
          />
        );
      })}
      {/* 👇️ only match this when no other routes match */}
      <Route path="*" element={<NotFound />} />
    </Routes>
  );
};

export default AppRoutes;
