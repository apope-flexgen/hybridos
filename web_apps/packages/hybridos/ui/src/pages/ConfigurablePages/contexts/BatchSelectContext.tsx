import {
  createContext, FC, ReactNode, useMemo, useState,
} from 'react';

export type BatchSelectContextType = {
  selectedAssets: string[];
  setSelectedAssets: React.Dispatch<React.SetStateAction<string[]>>;
};

interface Props {
  children: ReactNode;
}

export const BatchSelectContext = createContext<BatchSelectContextType | null>(null);

const BatchSelectProvider: FC<Props> = ({ children }) => {
  const [selectedAssets, setSelectedAssets] = useState<string[]>([]);

  const context = useMemo(
    () => ({
      selectedAssets,
      setSelectedAssets,
    }),
    [selectedAssets, setSelectedAssets],
  );

  return <BatchSelectContext.Provider value={context}>{children}</BatchSelectContext.Provider>;
};

export default BatchSelectProvider;
