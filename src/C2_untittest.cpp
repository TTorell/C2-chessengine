/*
 * C2_untittest.cpp
 *
 *  Created on: 29 jan. 2019
 *      Author: torsten
 */
#include "chesstypes.hpp"

int C2_unit_test::main_test(ostream& os)
{
  os << "###########################" << endl;
  os << "#         Movelist        #" << endl;
  os << "###########################" << endl << endl;
  os << "... default constructor ..." << endl;
  Movelist movelist;
  os << "nof_elements = " << movelist.cardinal() << endl;

  os << ".. into, cardinal, operator[] .." << endl;
  Position from(e, 2);
  Position to(e, 4);
  Move* move1 = new Move(from, to);
  Move* move2 = new Move(from, to);
  movelist.into(move1);
  movelist.into(move2);
  for (int i = 0; i < movelist.cardinal(); i++)
  {
    os << *movelist[i] << endl;
  }
  Position from2(d, 2);
  Position to2(d, 4);
  Movelist movelist2;
  Move* move3 = new Move(from2, to2);
  Move* move4 = new Move(from2, to2);
  movelist2.into(move3);
  movelist2.into(move4);

  os << ".. assignment operator and write .." << endl;
  movelist = movelist2;

  os << "... copy constructor ..." << endl;
  Movelist movelist3(movelist2);
  for (int i = 0; i < movelist3.cardinal(); i++)
  {
    os << *movelist3[i] << endl;
  }

  os << "...... destructor ......" << endl << endl;
  Movelist* dynamic_list = new Movelist(movelist2);
  delete dynamic_list;

  move3 = new Move(from2, to2);
  move4 = new Move(from2, to2);
  movelist2.into(move3);
  movelist2.into(move4);

  os << ".... first and next ..." << endl;
  Move* m = movelist2.first();
  while (m)
  {
    os << *m << endl;
    m = movelist2.next();
  }

  os << ".... into_as_last ...." << endl;
  Position from3(g, 1);
  Position to3(f, 3);
  Move* move5 = new Move(from3, to3);
  movelist2.into_as_last(move5);
  for (int i = 0; i < movelist2.cardinal(); i++)
  {
    os << *movelist2[i] << endl;
  }

  os << "........ out ........." << endl;
  m = movelist2.first();
  while (m)
  {
    if (m->get_from() == move3->get_from())
    {
      movelist2.out(m);
      delete m;
      break;
    }
    m = movelist2.next();
  }
  m = movelist2.first();
  while (m)
  {
    if (m->get_from() == move5->get_from())
    {
      movelist2.out(m);
      delete m;
      break;
    }
    m = movelist2.next();
  }
  movelist2.write(os) << endl;

  os << "..... empty and clear ......" << endl;
  os << movelist2.empty() << endl;
  movelist2.clear();
  os << movelist2.empty() << endl;

  os << "..... in_list ......" << endl;
  movelist2.into(move1);
  movelist2.into(move3);
  movelist2.into(move5);
  Move* m_to_find = 0;
  int index_to_find;
  m = movelist2.first();
  while (m)
  {
    os << *m << endl;
    if (m->get_from() == move3->get_from())
    {
      m_to_find = m;
      break;
    }
    m = movelist2.next();
  }
  os << movelist.in_list(m_to_find, &index_to_find) << endl;
  os << index_to_find << endl;
  os << movelist.in_list(move1, &index_to_find) << endl;
  os << index_to_find << endl;

  os << "###########################" << endl;
  os << "#         Movelog         #" << endl;
  os << "###########################" << endl << endl;

  os << "... default constructor ..." << endl;
  Movelog movelog;
  movelog.write(os);

  Position f1(e, 2);
  Position t1(e, 4);
  Position f2(e, 7);
  Position t2(e, 5);
  Position f3(g, 1);
  Position t3(f, 3);
  Position f4(b, 8);
  Position t4(c, 6);
  Position f5(f, 1);
  Position t5(c, 4);
  Move* m1 = new Move(f1, t1);
  Move* m2 = new Move(f2, t2);
  Move* m3 = new Move(f3, t3);
  Move* m4 = new Move(f4, t4);
  Move* m5 = new Move(f5, t5);
  movelog.into(m1);
  movelog.into(m2);
  movelog.into(m3);
  movelog.into(m4);
  movelog.into(m5);
  movelog.write(os) << endl;

  Movelog movelog2(black);
  movelog2.into(m2);
  movelog2.into(m3);
  movelog2.into(m4);
  movelog2.into(m5);
  movelog2.write(os) << endl;

  movelist = movelog2;
  movelog.write(os) << endl;

  movelog.set_col_to_start(white);
  movelog.into_as_first(m1);
  movelog.write(os) << endl;

  Movelist* ml = new Movelog(movelog);
  ml->write(os);
  delete ml;


  return 0;
}
