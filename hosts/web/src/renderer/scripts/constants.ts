import { MewoNameCheck } from "../../mewo/types";

const kIconSize = 14;

const kNameErrorMessages = {
  [MewoNameCheck.Empty]: "Name cannot be empty",
  [MewoNameCheck.TooLong]: "Name is too long",
  [MewoNameCheck.InvalidCharacter]: "Name has invalid characters",
  [MewoNameCheck.LeadingOrTrailing]: "Name cannot have leading or trailing whitespace or periods",
  [MewoNameCheck.Reserved]: "Name cannot use a reserved name",
  [MewoNameCheck.Taken]: "Name is already taken",
};

export { kIconSize, kNameErrorMessages };
