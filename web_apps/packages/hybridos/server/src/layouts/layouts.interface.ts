import { Layout } from "shared/types/dtos/layouts.dto"
import { User } from "src/users/dtos/user.dto"
import { LayoutsResponse } from "./responses"

export interface AddLayout {
  data: Layout[]
}

export interface ILayoutsService {
  getLayouts(): Promise<LayoutsResponse>
  postLayouts(data: AddLayout, user: User): Promise<LayoutsResponse>
}
