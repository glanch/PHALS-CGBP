# Hausarbeit
##Modelldefinition PHALS
#Natation

|Indizes und Mengen          |                                                                                  |
| --------------------------------------------------- | ------------------------------------------------------- |
|$i,j \ \ \in \mathcal{I}$                            | Menge der Coils                                         |       
|$k \in \mathcal{K}$  				      | Menge der parallelen Linien                             |
|$m \in \mathcal{M}_{ik}$ , $n\ \in \mathcal{M}_{jk}$ | Menge der möglichen Modi für Coil i und j auf Linie k   |


|Parameter                                            |                                                         |
| --------------------------------------------------- | ------------------------------------------------------- |
|$p_{ikm}$                                            | Bearbeitungsdauer von Coil i auf der Linie k in Modus m |
|$\alpha$                                             | Maximale Anzahl an verspäteten Coils                    |
|$d_i$                                                | Fälligkeitsdatum von Coil i                             |
|$c_{ijkmn}$                                          | Kosten für einen Stringer zwischen Coil i in Modus m und Coil |
|$t_{ijkm}$                                          |Bearbeitungsdauer von einem Stringer zwischen Coil i in Modus |

|Entscheidungsparameter                               |                                                         |
| --------------------------------------------------- | ------------------------------------------------------- |
| $X_{ijkmn} \in \{0,1\}$                             |  1, wenn Coil i in Modus m direkt vor Coil j in Modus n auf der Linie k produziert wird, 0 sonst |
| $Z_{i} \in \{0,1\}$                                 | 1, wenn Coil i Verspätung hat, 0 sonst                  |
| $S_i$ \geq 0$                                       | Startzeit der Bearbeitung von Coil i                    |

##Kompaktes Modell
'''math
Minimiere   \qquad  \sum_{i \in \mathcal{I}} \sum_{j \in \mathcal{I}} \sum_{k \in \mathcal{K}} \sum_{m \i \mathcal{M}_{ik}} \sum_{n \in \mathcal{M}_{jk}}  X_{ijkmn} \cdot c_{ijkmn} \\

s.t.\\

\sum_{j=1}^{I+1} \sum_{k \in \mathcal{K}} \sum_{m \in \mathcal{M}_{ik}} \sum_{n \in \mathcal{M}_{jk}} X_{ijkmn} = 1  && \forall \ i \in  \mathcal{I} \\



## Rendered pdf
# ba-thesis-paper
Rendered paper pdf can be accessed [here](https://gitlab.uni-hannover.de/christopher.glanderluh/or-ii-final-project/-/jobs/artifacts/main/raw/Assignment/Hausarbeit/Hausarbeit.pdf?job=paper).