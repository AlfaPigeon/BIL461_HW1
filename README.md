# BIL461_HW1

Hazırlayanlar:
1- Oğuz Kaan Özsoy   171101061
2- Yiğit Çanga       221111043

Kodun çalışma mekanizması şu şekildedir:

1- Herhangi bir client Server'e kendi id'sini yolluyor ve karşılığında server bir thread oluşturup içerisinde mailbox çalıştırıyor ve mailbox'un id'sini aynı client'e cevap olarak atıyor. Server her mailbox için farklı thread oluşturuyor çünkü recv fonksiyonu kullanılarak thread kitleniyor ve senkronizasyonun herhangi bir şekilde bozulması engelleniyor.

2- Client server'e mesajı gönderiyor. Mesajın içeriği long mtype, char mtext, int client_id ve int receiver_id şeklindedir. Mtype verisi ile client'in server'e yapmasını istediği şey yollanır. Bu işlemler 4 tanedir ve 1'den 4'e sırasıyla şu şekildedir: server'e kaydedilmek, mesaj göndermek, yanıt almak ve client'i sonlandırmaktır. Mtype 1 olarak gönderilirse client için özel bir thread içinde mailbox oluşturulur ve client'e bildirilir (ilk maddede açıklandığı şekilde gerçekleşir). Mtype 2 olarak gönderilirse aynı içerikte gönderilen receiver'e mtext içindeki mesajın gönderilmesi istenir. Mtype 3 client'e yanıt dönülmesi için kullanılır ve son olarak mtype 4 client'e son vermek için kullanılır.

Bu şekilde iki client arasında server kullanılarak haberleşme sağlanır.
