// A diagram item and its child relationships, matching the format from psm_tree.json.
export type DiagramItem = {
  id: string,
  children: DiagramItem[] | undefined
};
