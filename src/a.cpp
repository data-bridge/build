
string Contract::TricksAsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("LIN declarer not implemented");
      return "";

    case BRIDGE_FORMAT_PBN:
      return Contract::VulAsBBN();

    case BRIDGE_FORMAT_RBN:
      LOG("RBN declarer not implemented");
      return "";

    case BRIDGE_FORMAT_TXT:
      LOG("TXT declarer not implemented");
      return "";

    default:
      LOG("Other declarer formats not implemented");
      return "";
  }
}

