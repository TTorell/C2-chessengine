#ifndef _POSITION_READER
#define _POSITION_READER

class Position_reader
{
  protected:
    Game& _game;

  public:
    Position_reader(Game& game) :
        _game(game)
    {
    }

    virtual int read_position(const string& inputfile) = 0;

    virtual istream& read_position(istream& is) = 0;

    virtual ~Position_reader()
    {
    }
};

class FEN_reader: public Position_reader
{
  public:
    FEN_reader(Game& game) :
        Position_reader(game)
    {
    }

    ~FEN_reader()
    {
    }

    int parse_FEN_string(const string& FEN_string) const;
    int read_position(const string& inputfile);
    istream& read_position(istream& is)
    {
      return is;
    }

  private:
    const string get_infotext(const string& line);
    void parse_string();

};

#endif
