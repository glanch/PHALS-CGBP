# Hausarbeit
## Modelldefinition PHALS
# Natation

|Indizes und Mengen          |                                                                                  |
| --------------------------------------------------- | ------------------------------------------------------- |
|$i,j \ \ \in \mathcal{I}$                            | Menge der Coils                                         |       
|$i,j \ \ \in \mathcal{I}_s$                          | Menge der Coils mit Startcoil                           |       
|$i,j \ \ \in \mathcal{I}^e$                          | Menge der Coils mit Endcoil                             |       
|$i,j \ \ \in \mathcal{I}^e_s$                       | Menge der Coils mit Start- und Endcoil                  |       
|$k \in \mathcal{K}$  				      | Menge der parallelen Linien                             |
|$m \in \mathcal{M_{ik}},~n \in \mathcal{M_{jk}}$    | Menge der möglichen Modi für Coil i und j auf Linie k   |


|Parameter                                            |                                                         |
| --------------------------------------------------- | ------------------------------------------------------- |
|$p_{ikm}$                                            | Bearbeitungsdauer von Coil i auf der Linie k in Modus m |
|$\alpha$                                             | Maximale Anzahl an verspäteten Coils                    |
|$d_i$                                                | Fälligkeitsdatum von Coil i                             |
|$c_{ijkmn}$                                          | Kosten für einen Stringer zwischen Coil i in Modus m und Coil |
|$t_{ijkm}$                                          |Bearbeitungsdauer von einem Stringer zwischen Coil i in Modus |

|Entscheidungsparameter                               |                                                         |
| --------------------------------------------------- | ------------------------------------------------------- |
| $X_{p,ijmn}$                            	      |  1, wenn Coil i in Modus m direkt vor Coil j in Modus n auf der Linie k produziert wird, 0 sonst |
| $x^k_{p,ijmn}$                            	      |  Koeffizient, der Belegung von $X_{ijkmn}$ in Pricing-Problem von Linie $k$ in Extrempunkt $p$ angibt              |
| $\lambda^k_p$                                       | 1, wenn Muster ausgewählt				|
| $Z_{ik} \in \{0,1\}$                                 | 1, wenn Coil i Verspätung hat, 0 sonst                  |
| $z^k_{p,i} \in \{0,1\}$                               |  Koeffizient, der Belegung von $Z_{ik}$ in Pricing-Problem von Linie $k$ in Extrempunkt $p$ angibt                                                 |                                
| $S_{ik} \geq 0$                                        | Startzeit der Bearbeitung von Coil i                    |

|Dual Variablen                             |                                                         |
| --------------------------------------------------- | ------------------------------------------------------- |
|$\pi_{\alpha}$                                 | Dual Anzahl Verspätungen $\alpha$ Beschränkung                                                   |
|$\pi^{Z_{ik}}_\text{orig}$                                 | Dual Originalvariable Rekonstruktion $Z_{ik}$                                                 |
|$\pi^{i}_\text{part}$                                 |  Dual Coil-Mode-Partitionierung pro Coil $i$                                                |
|$\pi^{X_{ijkmn}}_\text{orig}$                                 | Dual X Originalvariable Rekonstruktion $X_{ijkmn}$                                                 |
|$\pi^{k}_\text{conv}$                                 |  Dual Convexification pro Pricing-Problem / Produktionslinie $k$                                                 |
## Master-Problem
![alt text](./pictures/master1.png)
## Pricing
Pricing-Problem für Produktionslinie $k$
![alt text](./pictures/pricer1.png)