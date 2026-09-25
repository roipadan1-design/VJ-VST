# כלי סטילס (לא חלק מהמנוע)

רנדרר קטן ב-Python שמריץ את **השיידרים האמיתיים** של הסצנות (`engine/Shaders/Instrument`) ואת שרשרת ה-FINISH וה-LOOK/FILM של המנוע (נקראת ישירות מ-`LookPass.cpp` ו-`FinishPass.cpp`). הוא מחקה גם את חישוב המאקרו והראוטים של `Modulation.cpp`. ככה צילמנו את כל התמונות בתיקייה `stills/`, על שרת לינוקס ובלי לבנות את המנוע.

זה כלי סקיצות: אין בו אודיו חי, מעברים או טריגרים, ו-bloom מקורב.

| קובץ | מה הוא עושה |
|---|---|
| `vjrender.py` | הרנדרר: סצנה, מודולציה, FINISH ו-LOOK, ומדידת בהירות (APL) |
| `audit.py`, `sheet.py` | 13 הסצנות ב-3 פלטות, ודפי קונטקט |
| `mock_util.py` | עזרים משותפים: רנדור סצנה ל-numpy ומעבר של תמונה סינתטית דרך ה-LOOK |
| `mock_memory.py` | **אב-טיפוס של "זיכרון המופע"**: היזכרות עם אובדן דורות, התקשות, קצה המסגרת, צירוף מחדש, התכנסות לטבעת |
| `mock_d23.py`, `mock_extra.py` | סטילס לכיוונים 2 ו-3, ותרשים אזור הגוף |

הרצה (בלינוקס בלי מסך, מתוך התיקייה הזו):

```
pip install moderngl numpy pillow scipy
xvfb-run -a python3 audit.py        # התמונות נכתבות ל-out/
xvfb-run -a python3 mock_memory.py
```

ב-Windows אפשר להריץ `python audit.py` ישירות. צריך OpenGL 3.3 ומעלה, וה-Iris Xe עומד בזה.
